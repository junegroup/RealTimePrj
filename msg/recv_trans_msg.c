#include <SocketMessage.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int send_msg_to_server(int socket_fd, int msg_type)
{
	int iError;
	
	switch(msg_type) {
	case MSG_IDENTIFY:{
		char msg_body[100];
		SocketMessage send_msg;
		SocketMessage recv_msg;

		send_msg.type = MSG_IDENTIFY;
		send_msg.srcDevId = DEV_ID;
		send_msg.desDevId = SER_ID;
		sprintf(msg_body, "{\"devId\":\"%d\"}", DEV_ID);
		send_msg.bodyLen = strlen(msg_body);
		send_msg.body = msg_body;
		
		iError = sendSocketMessage(socket_fd, send_msg);
		if(iError) {
			printf("MSG_IDENTIFY:firstTimeConnect ==> sendSocketMessage failed!\n");
			return iError;
		}
		recv_msg = recvSocketMessage(socket_fd);
		if(iError) {
			printf("MSG_IDENTIFY:firstTimeConnect ==> recvSocketMessage failed!\n");
			return iError;
		}
		showSocketMessage(recv_msg);

		/* return msg:ok to server */
		send_msg.type = MSG_SETTIMERESULT;
		send_msg.srcDevId = DEV_ID;
		send_msg.desDevId = SER_ID;
		sprintf(msg_body, "{\"msg\":\"ok\"}");
		send_msg.bodyLen = strlen(msg_body);
		send_msg.body = msg_body;
		iError = sendSocketMessage(socket_fd, send_msg);
		if(iError) {
			printf("MSG_IDENTIFY:firstTimeConnect ==> sendSocketMessage failed!\n");
			return iError;
		}

		break;
	}
	case MSG_SCANDATA:{
//		char msg_body[100];
//		SocketMessage send_msg;
		SocketMessage recv_msg;

		/* add your content will send */

		/* add done */
		recv_msg = recvSocketMessage(socket_fd);
		showSocketMessage(recv_msg);

		
		break;
	}
	case MSG_STATIONLIVE:{
		char msg_body[100];
		SocketMessage send_msg;
		//SocketMessage recv_msg;

		send_msg.type = MSG_STATIONLIVE;
		send_msg.srcDevId = DEV_ID;
		send_msg.desDevId = SER_ID;
		sprintf(msg_body, "{\"msg\":\"ok\"}");
		send_msg.bodyLen = strlen(msg_body);
		send_msg.body = msg_body;
		
		iError = sendSocketMessage(socket_fd, send_msg);
		if(iError) {
			printf("MSG_STATIONLIVE:firstTimeConnect ==> sendSocketMessage failed!\n");
			return iError;
		}
#if 0
		recv_msg = recvSocketMessage(socket_fd);
		if(iError) {
			printf("MSG_STATIONLIVE:firstTimeConnect ==> recvSocketMessage failed!\n");
			return iError;
		}
		showSocketMessage(recv_msg);
#endif
		break;
	}
	default:
		printf("send_msg_to_server ==> something wrong!\n");
	}
	
	return 0;
}

