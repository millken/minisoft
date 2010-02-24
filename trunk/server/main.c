#include "net.h"

#define CRLF "\r\n"

void accept_cb (SOCKET sclient)
{
	char *s = "\x1b[0mWelcome to my world!\r\n";
	send(sclient, s, strlen(s), 0);
}

void recv_cb (SOCKET sclient, char *msg, int len)
{
	if (strcmp(msg, CRLF)) {
		strcat (msg, CRLF);
		send (sclient, msg, len+sizeof(CRLF)+1, 0);
	}
}

int main()
{
	server_run (8888, accept_cb, recv_cb);

    return 0;
}
