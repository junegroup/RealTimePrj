#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <signal.h>
#include <alsa/asoundlib.h>
#include <pthread.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netinet/tcp.h>
#include <lame/lame.h>
#include <SocketMessage.h>
#include <command.h>

#define INBUFF_SIZE     64
#define MP3BUFF_SIZE    (int) (1.25 * INBUFF_SIZE) + 7200


static int run_flag = 0;
static snd_pcm_t *capture_handle;
static snd_pcm_t *playback_handle;
static lame_global_flags *gfp = NULL;
static unsigned int rate = 8000;


static snd_pcm_t *open_sound_dev(snd_pcm_stream_t type, unsigned int rate)
{
	int err;
	snd_pcm_t *handle;
	snd_pcm_hw_params_t *hw_params;
	//unsigned int rate = 8000;

	if ((err = snd_pcm_open(&handle, "default", type, 0)) < 0)
		return NULL;
	   
	if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0) {
		fprintf (stderr, "cannot allocate hardware parameter structure (%s)\n",
			 snd_strerror (err));
		return NULL;
	}
			 
	if ((err = snd_pcm_hw_params_any (handle, hw_params)) < 0) {
		fprintf (stderr, "cannot initialize hardware parameter structure (%s)\n",
			 snd_strerror (err));
		return NULL;
	}

	if ((err = snd_pcm_hw_params_set_access (handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
		fprintf (stderr, "cannot set access type (%s)\n",
			 snd_strerror (err));
		return NULL;
	}

	if ((err = snd_pcm_hw_params_set_format (handle, hw_params, SND_PCM_FORMAT_S16_LE)) < 0) {
		fprintf (stderr, "cannot set sample format (%s)\n",
			 snd_strerror (err));
		return NULL;
	}

	if ((err = snd_pcm_hw_params_set_rate_near (handle, hw_params, &rate, 0)) < 0) {
		fprintf (stderr, "cannot set sample rate (%s)\n",
			 snd_strerror (err));
		return NULL;
	}

	if ((err = snd_pcm_hw_params_set_channels (handle, hw_params, 2)) < 0) {
		fprintf (stderr, "cannot set channel count (%s)\n",
			 snd_strerror (err));
		return NULL;
	}

	if ((err = snd_pcm_hw_params (handle, hw_params)) < 0) {
		fprintf (stderr, "cannot set parameters (%s)\n",
			 snd_strerror (err));
		return NULL;
	}

	snd_pcm_hw_params_free(hw_params);

	return handle;
}

static int init_lame_tools(void)
{
	int err = 0;
	
	gfp = lame_init();
	//printf("init_lame_tools ==> gfp\n");
	if(gfp == NULL) {
		printf("lame_init failed\n");
		err = -1;
	}

	lame_set_in_samplerate(gfp, rate);
	lame_set_num_channels(gfp, 2);
	lame_set_brate(gfp, rate / 1000);
	lame_set_quality(gfp, 5);

	err = lame_init_params(gfp);
	if(err < 0) {
		printf("lame_init_params returned\n");
		err = -1;
	}

	return err;
}

static void close_sound_dev(snd_pcm_t *handle)
{
	snd_pcm_close (handle);
}

static snd_pcm_t *open_capture(unsigned int rate)
{
    return open_sound_dev(SND_PCM_STREAM_CAPTURE, rate);
}

static snd_pcm_t *open_playback(unsigned int rate)
{
    return open_sound_dev(SND_PCM_STREAM_PLAYBACK, rate);
}

int init_snd_for_playback(unsigned int rate)
{
	int err = 0;
	playback_handle = open_playback(rate);
	if (!playback_handle) {
		fprintf (stderr, "cannot open for playback\n");
	    return -1;
	}

	if ((err = snd_pcm_prepare (playback_handle)) < 0) {
		fprintf (stderr, "cannot prepare audio interface for use (%s)\n",
			 snd_strerror (err));
		return -1;
	}

	return err;
}

static int init_snd_for_capture(unsigned int rate)
{
	int err = 0;

	capture_handle = open_capture(rate);
    if (!capture_handle) {
		fprintf (stderr, "cannot open for capture\n");
        return -1;
    }

	if ((err = snd_pcm_prepare (capture_handle)) < 0) {
		fprintf (stderr, "cannot prepare audio interface for use (%s)\n",
			 snd_strerror (err));
		return -1;
	}

	return err;
}


void set_runflag_for_send_audiodata(int flag)
{
	if(flag)
		run_flag = 1;
	else
		run_flag = 0;
}

int send_audio_data_to_server_by_mp3stream(int realtime_flag, char *server_ipaddr, int tcp_socketclient)
{
	int mp3_bytes = 0;
	int rError;
//	int udp_socketclient;
//	int iSendLen = 0;// iAddrLen = sizeof(struct sockaddr);
//	struct sockaddr_in tSocketServerAddr;

	char buff_avi[INBUFF_SIZE * 2] = {0};
	char mp3_buff[MP3BUFF_SIZE];

#if 0
	udp_socketclient = socket(AF_INET, SOCK_DGRAM, 0);
	tSocketServerAddr.sin_family = AF_INET;
	tSocketServerAddr.sin_port   = htons(UDP_PORT);

	if(0 == inet_aton(server_ipaddr, (struct in_addr *)&tSocketServerAddr.sin_addr)) {
		printf("send_audio_data_to_server_by_mp3stream ==> invalid server_ip\n");
		return -1;
	}
	memset(tSocketServerAddr.sin_zero, 0, 8);

	rError = connect(udp_socketclient, (const struct sockaddr *)&tSocketServerAddr, sizeof(struct sockaddr));
	if(rError == -1) {
		printf("connect udp error!\n");
		return -1;
	}
#endif
	if(realtime_flag) {
		char *buffer_to_send;
		
		/* First, initialize mp3 code */
		rError = init_lame_tools();
		if(rError) {
			printf("send_audio_data_to_server_by_mp3stream ==> init_lame_tools Error!\n");
			return -1;   // maybe fix
		}

		rError = init_snd_for_capture(8000);
		if(rError) {
			printf("send_audio_data_to_server_by_mp3stream ==> init_snd_for_capture Error!\n");
			return -1;   // maybe fix
		}

		rError = init_snd_for_playback(8000);

		while(run_flag) {
			int i;
			short *testdata;

			/* recv data from client and print it */
			rError = snd_pcm_readi(capture_handle, buff_avi, 32);
			if (rError == -EPIPE) {
				fprintf (stderr, "read from audio interface failed (%s)\n",
				 		snd_strerror (rError));
				snd_pcm_prepare(capture_handle);
			}else if(rError != 32){
				fprintf(stderr, "read from audio interface failed (%s)\n",
					 	snd_strerror (rError));
				//close(udp_socketclient);
				return -1;
			}

			//snd_pcm_writei(playback_handle, buff_avi, 32);
 
			mp3_bytes = lame_encode_buffer_interleaved(gfp, (short *)buff_avi, 32, (unsigned char *)mp3_buff, MP3BUFF_SIZE);
			if(mp3_bytes < 0) {
				printf("lame_encode_buffer_interleaved failed! st = %d\n", mp3_bytes);
				//close(udp_socketclient);
				return (-1);
			}else if(mp3_bytes > 0) {
				//printf("mp3_bytes = %d\n", mp3_bytes);
				buffer_to_send = base64_encode((const char*)mp3_buff, mp3_bytes);
				if(!buffer_to_send) {
					printf("send_audio_data_to_server_by_mp3stream ==> base64_encode!\n");
					return -1;
				}

				//printf("base64code: %s\n", buffer_to_send);

				SocketMessage audio_msg;
				char msg_body[1024];
				audio_msg.type = MSG_AUDIODATARESULT;
				audio_msg.srcDevId = DEV_ID;
				audio_msg.desDevId = DATA_ID;
				sprintf(msg_body, "{\"frequence\":\"%.1f\", \"msg\":\"ok\",\"frameBase64\":\"%s\"}", 99.8, buffer_to_send);
				audio_msg.bodyLen = strlen(msg_body);
				audio_msg.body = msg_body;

				//printf("msg_body: %s\n", msg_body);
				
				rError = sendSocketMessage(tcp_socketclient, audio_msg);
				if(rError) {
					printf("send_audio_data_to_server_by_mp3stream ==> sendSocketMessage failed!\n");
					//return rError;
				}

				base64_release(buffer_to_send);

			}
		}
		
		close_sound_dev(capture_handle);
		//close(udp_socketclient);
		lame_close(gfp);
	}
#if 0
	else {  /* section mode */
		char command[100];
		int loop_time = total_time / 5;
		int n = 0;
		int fd;
		struct stat mp3_stat;
		if(loop_time <= 0) {
			printf("Sorry, time something wrong!\n");
			close(server_fd);
			exit(-1);
		}

		while(loop_time--) {
			sprintf(command, "arecord -f cd ./arecord_data/%s%d.wav -d 5", "record_data-", n);
			err = system(command);
			
			sprintf(command, "lame ./arecord_data/%s%d.wav", "record_data-", n);
			err = system(command);
			
			sprintf(command, "rm ./arecord_data/%s%d.wav", "record_data-", n);
			err = system(command);

			sprintf(command, "./arecord_data/%s%d.mp3", "record_data-", n);
			fd = open(command, O_RDONLY);
			if(fd < 0) {
				printf("open mp3 file failed\n");
				close(server_fd);
				exit(-1);
			}

			err = fstat(fd, &mp3_stat);
			if(err) {
				printf("fstat error!\n");
				close(server_fd);
				exit(-1);
			}
			printf("%s size is: %d\n", command, (int)mp3_stat.st_size);

			iSendLen = send(server_fd, &mp3_stat.st_size, sizeof(mp3_stat.st_size), 0);
			if(iSendLen <= 0) {
				printf("error: send file size failed(iSendLen = %d)\n", iSendLen);
				close(server_fd);
				return -1;
			}

			char file_buffer[1000];
			unsigned int rd_len;
			unsigned int size = mp3_stat.st_size;

			while(size) {
				rd_len = read(fd, file_buffer, 1000);
				//printf("rd_len = %d\n", rd_len);
				iSendLen = send(server_fd, file_buffer, rd_len, 0);
				size -= rd_len;
			#if 0
				if(iSendLen <= 0) {
					close(server_fd);
					return -1;
				}else if(iSendLen == 1000) {
					size -= rd_len;
					continue;
				}else
					size = 0;
			#endif
			}

			close(fd);
			
			n++;
		}
	}
#endif

	return 0;
}

