#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <SocketMessage.h>
#include <sys/types.h>
#include <arpa/inet.h>

typedef struct {
	char type;
	long long srcDevId;
	long long desDevId;
	int bodyLen;
}__attribute__((packed)) MessageHeader;

/**
 * change host bytes to network bytes
 */
static long long htonll(long long num)
{
	unsigned char *p, p0, p1, p2, p3;

	if(htons(1) == 1)
		return num;

	p = (unsigned char *)&num;
	p0 = p[0];
	p1 = p[1];
	p2 = p[2];
	p3 = p[3];
	p[0] = p[7];
	p[1] = p[6];
	p[2] = p[5];
	p[3] = p[4];
	p[4] = p3;
	p[5] = p2;
	p[6] = p1;
	p[7] = p0;

	return num;
}

/**
 * change network bytes to host bytes
 */
static long long ntohll(long long num)
{
	unsigned char *p, p0, p1, p2, p3;

	if(ntohs(1) == 1)
		return num;

	p = (unsigned char *)&num;
	p0 = p[0];
	p1 = p[1];
	p2 = p[2];
	p3 = p[3];
	p[0] = p[7];
	p[1] = p[6];
	p[2] = p[5];
	p[3] = p[4];
	p[4] = p3;
	p[5] = p2;
	p[6] = p1;
	p[7] = p0;

	return num;
}

int sendSocketMessage(int socket_fd, SocketMessage msg)
{
	int rError;
	MessageHeader msgHeader;

	msgHeader.type = msg.type;
	msgHeader.srcDevId = htonll(msg.srcDevId);
	msgHeader.desDevId = htonll(msg.desDevId);
	msgHeader.bodyLen = htonl(msg.bodyLen);

	/* First of all, send msg header to host */
	rError = send(socket_fd, (char *)&msgHeader, sizeof(msgHeader), 0);
	if(rError < 0) {
		printf("sendSocketMessage ==> send msgHeader error!\n");
		return rError;
	}
	
	/* then send msg body to host server */
	rError = send(socket_fd, msg.body, msg.bodyLen, 0);
	if(rError < 0) {
		printf("sendSocketMessage ==> send msg.body error!\n");
		return rError;
	}

	return 0;
}

#if 0
int UDP_sendSocketMessage(int socket_fd, SocketMessage msg, const struct sockaddr *sockaddr, int addrLen)
{
	int rError;
	MessageHeader msgHeader;

	msgHeader.type = msg.type;
	msgHeader.srcDevId = htonll(msg.srcDevId);
	msgHeader.desDevId = htonll(msg.desDevId);
	msgHeader.bodyLen = htonl(msg.bodyLen);

	/* First of all, send msg header to host */
	rError = sendto(socket_fd, (char *)&msgHeader, sizeof(msgHeader), 0, sockaddr, addrLen);
	if(rError < 0) {
		printf("sendSocketMessage ==> send msgHeader error!\n");
		return rError;
	}
	
	/* then send msg body to host server */
	rError = sendto(socket_fd, msg.body, msg.bodyLen, 0, sockaddr, addrLen);
	if(rError < 0) {
		printf("sendSocketMessage ==> send msg.body error!\n");
		return rError;
	}

	return 0;
}
#endif


int UDP_sendSocketMessage(int socket_fd, SocketMessage msg)
{
	int rError;
	MessageHeader msgHeader;

	msgHeader.type = msg.type;
	msgHeader.srcDevId = htonll(msg.srcDevId);
	msgHeader.desDevId = htonll(msg.desDevId);
	msgHeader.bodyLen = htonl(msg.bodyLen);

	/* First of all, send msg header to host */
	rError = send(socket_fd, (char *)&msgHeader, sizeof(msgHeader), 0);
	if(rError < 0) {
		printf("sendSocketMessage ==> send msgHeader error!\n");
		return rError;
	}
	
	/* then send msg body to host server */
	rError = send(socket_fd, msg.body, msg.bodyLen, 0);
	if(rError < 0) {
		printf("sendSocketMessage ==> send msg.body error!\n");
		return rError;
	}

	return 0;
}



/**
 * read bytes from socket_fd
 */
static int readByte(int socket_fd, char *pBuffer, int len)
{
	int total = 0;

	while(total < len) {
		int n = recv(socket_fd, pBuffer + total, len - total, 0);
		if(n < 0) {
			printf("readByte ==> recv error!\n");
			return n;
		}

		total += n;
	}

	return 0;
}

SocketMessage recvSocketMessage(int socket_fd)
{
	int rError;
	SocketMessage msg;
	MessageHeader msgHeader;
	
	/* First, recv msg header */
	rError = readByte(socket_fd, (char *)&msgHeader, sizeof(msgHeader));
	if(rError)
		printf("recvSocketMessage ==> readByte error!\n");

	msg.type = msgHeader.type;
	msg.srcDevId = ntohll(msgHeader.srcDevId);
	msg.desDevId = ntohll(msgHeader.desDevId);
	msg.bodyLen = ntohl(msgHeader.bodyLen);
	msg.body = (char *)malloc(msg.bodyLen);

	/* then recv msg body */
	rError = readByte(socket_fd, msg.body, msg.bodyLen);
	if(rError) 
		printf("recvSocketMessage ==> readByte2 error!\n");

	return msg;
}

void showSocketMessage(SocketMessage msg)
{
	puts("\n+++++++++++++++++++msg++++++++++++++++");
	printf("type     : %d\n", msg.type);
	printf("srcDevId : %lld\n", msg.srcDevId);
	printf("desDevId : %lld\n", msg.desDevId);
	printf("bodyLen  : %d\n", msg.bodyLen);
	printf("body     : %s\n", msg.body);
	puts("---------------------------------------\n");
}

int initAndConnectToServer(int *server_fd)
{
	int iError;
	*server_fd = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in serverAddr;

	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(TCP_PORT);
	/* the server ip */
	serverAddr.sin_addr.s_addr = inet_addr("61.139.104.215");

	iError = connect(*server_fd, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
	if(iError < 0) {
		printf("initAndConnectToServer => connect error!\n");
		return iError;
	}

	return 0;
}

