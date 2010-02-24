/**
 * lib: libws2_32.a
 */

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <winsock2.h>
#include <process.h>
#include "net.h"

static RecvCB g_recvf = NULL;

static void reset_iodata (IOdata *iodata, SOCKET sclient)
{
	if (!iodata) return;

	memset(iodata, 0, sizeof(IOdata));
	iodata->buffer.len = MSGSIZE;
	iodata->buffer.buf = iodata->msg;
	WSARecv(sclient,
		&iodata->buffer,
		1,
		&iodata->rcvlen,
		&iodata->flags,
		&iodata->overlap,
		NULL);
}

static void workthread (void *param)
{
	HANDLE comport = (HANDLE)param;
	char *msg;
	DWORD msglen;
	SOCKET sclient;
	IOdata *iodata = NULL;

	while (1) {
		GetQueuedCompletionStatus (comport, &msglen, &sclient,
			(LPOVERLAPPED *)&iodata, INFINITE);
		if (msglen == 0xFFFFFFFF) {
			printf ("error\n");
			return;
		}

		if (msglen == 0) {
			printf ("close\n");
			closesocket (sclient);
			free(iodata);
			return;
		}

		msg = iodata->msg;
		msg[msglen] = '\0';

		if (g_recvf) g_recvf(sclient, msg, msglen);

		/* Launch another asynchronous operation for sClient */
		reset_iodata (iodata, sclient);
	}
}

int server_run (unsigned short port, AcceptCB acceptf, RecvCB recvf)
{
	WSADATA wsadata;
	HANDLE comport;
	SYSTEM_INFO sinfo;
	SOCKET slisten, sclient;
	SOCKADDR_IN local, client;
	int addrsize = sizeof(SOCKADDR_IN);
	IOdata *iodata = NULL;
	int i;

	g_recvf = recvf;

	if (WSAStartup(0x0202, &wsadata))
		printf ("wsastartup error!\n");

	comport = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	printf ("completport: %u\n", comport);

	GetSystemInfo(&sinfo);
	printf ("cpu: %lu\n", sinfo.dwNumberOfProcessors);

	for (i = 0; i < sinfo.dwNumberOfProcessors; i++)
		_beginthread(workthread, 0, (void *)comport);

	slisten = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	local.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	local.sin_family = AF_INET;
	local.sin_port = htons(port);
	bind(slisten, (struct sockaddr *)&local, sizeof(SOCKADDR_IN));
	listen (slisten, LISTENMAX);

	while (TRUE) {
		// Accept a connection
		sclient = accept(slisten, (struct sockaddr *)&client, &addrsize);
		printf("Accepted client:%s:%d, %d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port), sclient);
		// Associate the newly arrived client socket with completion port

		CreateIoCompletionPort((HANDLE)sclient, comport, (DWORD)sclient, 0);

		// Launch an asynchronous operation for new arrived connection
		iodata = (IOdata *)malloc(sizeof(IOdata));
		reset_iodata (iodata, sclient);

		if (acceptf)
			acceptf (sclient);
	}

	/* end */
    PostQueuedCompletionStatus(comport, 0xFFFFFFFF, 0, NULL);
    CloseHandle(comport);
    closesocket(slisten);
    WSACleanup();

    return 0;
}

