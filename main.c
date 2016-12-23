#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <command.h>
#include <signal.h>
#include <cJSON.h>
#include <SocketMessage.h>


/* global server socket */
static int server_fd;
static pthread_mutex_t LCK_only_one_pthread_trans_audiodata;
static int atom_trans_flag;
static int pthread_exit_params = 0;

static void sig_heartbeat(int signo)
{
	int iErr;

	iErr = send_msg_to_server(server_fd, MSG_STATIONLIVE);
	if(iErr)
		printf("sig_heartbeat error!\n");

	alarm(10);
}

static void *trans_audio_data_func(void *arg)
{
	
	printf("sent a frame audio data!\n");

	pthread_exit_params = send_audio_data_to_server_by_mp3stream(1, "61.139.104.215", server_fd);
	if(pthread_exit_params) {
		printf("pthread: trans_audio_data_func ==> send_audio_data_to_server_by_mp3stream wrong!\n");
		pthread_exit_params = -1;
	}

	printf("pthread finished!\n");
	
	pthread_exit(&pthread_exit_params);
}

int main(int argc, char **argv)
{
	int iErr;
	pthread_t trans_audiodata_pid;

	if(signal(SIGALRM, sig_heartbeat) == SIG_ERR) {
		printf("signal SIGALRM error!\n");
		exit(-1);
	}

	iErr = initAndConnectToServer(&server_fd);
	if(iErr) {
		printf("main ==> initAndConnectToServer error!\n");
		exit(-1);
	}

	iErr = send_msg_to_server(server_fd, MSG_IDENTIFY);
	if(iErr){
		printf("main ==> send_msg_to_server failed!\n");
		exit(-1);
	}

	alarm(10); /* setup MSG_STATIONLIVE */

	pthread_mutex_init(&LCK_only_one_pthread_trans_audiodata, NULL);

	while(1){
		int iError;
		SocketMessage recv_msg;
		recv_msg = recvSocketMessage(server_fd);

		showSocketMessage(recv_msg);

		switch (recv_msg.type) {
		case MSG_AUDIODATA: {
			double freq;
			freq = get_frequence_from_json(recv_msg.body);

			if(0 == pthread_mutex_trylock(&LCK_only_one_pthread_trans_audiodata)){
				if(!atom_trans_flag) {
					atom_trans_flag = 1;
					set_runflag_for_send_audiodata(1);
					iError = pthread_create(&trans_audiodata_pid, NULL, trans_audio_data_func, NULL);
					if(iError) {
						printf("pthread_create error!\n");
						continue;
					}
				}
				pthread_mutex_unlock(&LCK_only_one_pthread_trans_audiodata);
			}

			break;
		}
		case MSG_STOPAUDIODATA: {
			
			if(0 == pthread_mutex_trylock(&LCK_only_one_pthread_trans_audiodata)) {
				if(atom_trans_flag) {
					atom_trans_flag = 0;
					set_runflag_for_send_audiodata(0);
				}
				pthread_mutex_unlock(&LCK_only_one_pthread_trans_audiodata);
			}
			break;
		}
		default:
			printf("----");
		}
	}

	pthread_mutex_destroy(&LCK_only_one_pthread_trans_audiodata);
	close(server_fd);

	return 0;
}


