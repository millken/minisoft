#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <winsock2.h>
#include <process.h>

int main()
{
	WSADATA wsadata;
	unsigned ip;
	SOCKET sserver;
	struct sockaddr_in saddr;
	int addrsize;
	int err;

	if (WSAStartup(0x0202, &wsadata))
		printf ("wsastartup error!\n");

	ip = inet_addr ("127.0.0.1");
	if (ip == -1)
		printf ("Invalid IP address.\n");
	printf ("%u\n", ip);

    sserver = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);
	printf ("socket: %d\n", sserver);

	memset (&saddr, 0, sizeof(saddr));
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(8888);
	saddr.sin_addr.s_addr = ip;

	err = connect (sserver, (struct sockaddr *)&saddr, sizeof(saddr));
    printf ("connect: %d\n", err);

	char *text = "asdf";
    err = send(sserver, text, strlen(text)+1, 0);
	printf ("%d\n", err);

	shutdown (sserver, SD_BOTH);
	closesocket (sserver);
	WSACleanup();

    return 0;
}
