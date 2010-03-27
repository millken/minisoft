#include <stdio.h>
#include <io.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <winsock.h>
#include <curl/curl.h>
#include <assert.h>
#include "url.h"

#define closefile(p) if(p){fclose(p);p=NULL;}
#define freestring(s) if(s){free(s);s=NULL;}
#define closehandle(h) if(h){curl_easy_cleanup(h);h=NULL;}

#define yes 1
#define no 0
#define null ((void *)0)

#define URLMAX 100

typedef struct {
	int id;
	char *url;
	FILE *file;
	CURL *handle;
} urow;

static urow g_list[URLMAX];
static int g_len;

int url_add (int id, char *url)
{
	urow *u;
	char *newurl;

	if (id < 0 || !url || g_len >= URLMAX)
		return no;

	newurl = (char *)malloc(strlen(url)+1);
	if (!newurl)
		return no;
	strcpy (newurl, url);

	u = &g_list[g_len];

	u->id = id;
	u->url = newurl;
	u->file = null;
	u->handle = null;

	g_len++;

	return yes;
}

void url_clear ()
{
	urow *u;
	int i;

	for (i = 0; i < g_len; i++) {
		u = &g_list[i];
		u->id = 0;
		freestring (u->url);
		closefile (u->file);
		closehandle (u->handle);
	}
	g_len = 0;
}

void url_test ()
{
	int i;
	urow *u;

	for (i = 0; i < g_len; i++) {
		u = &g_list[i];
		printf ("id: %d, url: %s\n", u->id, u->url);
	}
}

static size_t
savepage (void *buffer, size_t size, size_t count, void *userp)
{
	return fwrite(buffer, size, count, g_list[(int)userp].file);
}

void url_download ()
{
	/* 初始化 */
	curl_global_init(CURL_GLOBAL_WIN32);
	CURLM *multi_handle = NULL;
	int running_handle_count;
	int i;

	multi_handle = curl_multi_init();

	mkdir (DOWNLOADDIR); /* 创建临时目录 */

	for (i = 0; i < g_len; i++) {
		urow *u = &g_list[i];
		char filename[256];

		sprintf (filename, "%s/%d.xml", DOWNLOADDIR, u->id);

		/* 如果文件存在但删除失败, pass */
		if (!access(filename, 0) && unlink(filename))
			continue;

		u->file = fopen(filename, "ab"); /* 二进制追加可读写 */
		if (!u->file)
			continue;

		u->handle = curl_easy_init();

		curl_easy_setopt(u->handle, CURLOPT_URL, u->url);
		curl_easy_setopt(u->handle, CURLOPT_WRITEFUNCTION, savepage);
		curl_easy_setopt(u->handle, CURLOPT_WRITEDATA, (void *)i);

		curl_multi_add_handle(multi_handle, u->handle);
	}

	while (CURLM_CALL_MULTI_PERFORM == curl_multi_perform(multi_handle, &running_handle_count))
	{
		printf ("running handle count: %d\n", running_handle_count);
		Sleep(1);
	}

	while (running_handle_count)
	{
		struct timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 0;

		int max_fd;
		fd_set fd_read;
		fd_set fd_write;
		fd_set fd_except;

		FD_ZERO(&fd_read);
		FD_ZERO(&fd_write);
		FD_ZERO(&fd_except);

		curl_multi_fdset(multi_handle, &fd_read, &fd_write, &fd_except, &max_fd);
		int return_code = select(max_fd + 1, &fd_read, &fd_write, &fd_except, &tv);
		if (SOCKET_ERROR == return_code)
		{
			printf ("select error\n");
			break;
		}
		else
		{
			while (CURLM_CALL_MULTI_PERFORM == curl_multi_perform(multi_handle, &running_handle_count))
				printf ("running handle count: %d\n", running_handle_count);
		}

		Sleep(1);
	}

	/* 释放资源 */
	url_clear ();
	curl_multi_cleanup(multi_handle);
	curl_global_cleanup();
}
