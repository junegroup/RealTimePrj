#ifndef COMMAND_H_
#define COMMAND_H_

char *base64_encode(const char* data, int data_len);
char *base64_decode(const char *data, int data_len);
void base64_release(char *buf);
void set_runflag_for_send_audiodata(int flag);

int send_audio_data_to_server_by_mp3stream(int realtime_flag, char *server_ipaddr, int tcp_socketclient);

#endif

