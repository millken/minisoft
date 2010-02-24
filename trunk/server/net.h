#ifndef _NET_H
#define _NET_H

#include <windows.h>
#include <winsock2.h>
#include <process.h>

#define LISTENMAX 1024
#define PORT 8888
#define MSGSIZE 1024

typedef struct {
	WSAOVERLAPPED  overlap;
	WSABUF         buffer;
	char           msg[MSGSIZE];
	DWORD          rcvlen;
	DWORD          flags;
} IOdata;

typedef void (*AcceptCB)(SOCKET sclient);
typedef void (*RecvCB)(SOCKET sclient, char *msg, int msglen);

int server_run (unsigned short port, AcceptCB, RecvCB);


#endif
