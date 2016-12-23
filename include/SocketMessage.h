#ifndef SOCKETMESSAGE_H_
#define SOCKETMESSAGE_H_

#include <sys/types.h>
#include <sys/socket.h>

/* this port is used to R/T messages */
#define TCP_PORT 1234
#define UDP_PORT 1235

#define DEV_ID  5103001
#define SER_ID  0
#define DATA_ID 5103

/**
 * 结构体SocketMessage已经取消了字节对齐
 */
typedef struct {
	char type;
	long long srcDevId;
	long long desDevId;
	int bodyLen;
	char *body;
}__attribute__ ((packed)) SocketMessage;

/* msg type */
#define MSG_IDENTIFY   				10
#define MSG_SETTIME      			11
#define MSG_SETTIMERESULT   		12
#define MSG_STATIONLIVE				13
#define MSG_SERVERLIVE				14
#define MSG_SCANDATA				15
#define MSG_SCANDATARESULT			16
#define MSG_SETFREQUENCE			17
#define MSG_SETFREQUENCERESULT		18
#define MSG_AUDIODATA				19
#define MSG_AUDIODATARESULT			20
#define MSG_AUDIODATAVEHICLE		21
#define MSG_AUDIODATAVEHICLERESULT 	22
#define MSG_STOPAUDIODATA			23
#define MSG_STOPAUDIODATARESULT		24
#define MSG_AUDIOFILELIST			25
#define MSG_AUDIOFILELISTRESULT		26
#define MSG_AUDIOFILEDATA			27
#define MSG_AUDIOFILEDATARESULT		28

/* function declaration */
int sendSocketMessage(int socket_fd, SocketMessage msg);
SocketMessage recvSocketMessage(int socket_fd);
void showSocketMessage(SocketMessage msg);
int initAndConnectToServer(int *server_fd);
int send_msg_to_server(int socket_fd, int msg_type);
//int UDP_sendSocketMessage(int socket_fd, SocketMessage msg, const struct sockaddr *sockaddr, int addrLen);
int UDP_sendSocketMessage(int socket_fd, SocketMessage msg);


#endif /* SOCKETMESSAGE_H_ */
