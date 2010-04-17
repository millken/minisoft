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
#define mfree(p) if(p)free(p)
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

static char g_userdir[1280] = {"u/0"};
static char g_downdir[1480] = {"u/0/download"};

static char g_postresult[512]; /* 登录后接收post结果 */
static CRITICAL_SECTION g_cs;

void url_init ()
{
	InitializeCriticalSection (&g_cs);
}

void url_close ()
{
	DeleteCriticalSection (&g_cs);
}

static char *nexttok (char *s, const char split, char **out)
{
	char *p;

	while (*s && (*s==split || *s==' ' || *s=='\t'))
		s++;

	*out = s;

	for (p = s; *p && *p!=split; p++);

	s = *p ? p + 1 : p; /* next start */
	*p = '\0';

	return s;
}

static char *dump (char *s)
{
	char *buf;

	s = s ? s : "";

	buf = (char *)malloc (strlen(s)+1);
	strcpy (buf, s);

	return buf;
}



struct user *url_newuser ()
{
	struct user *u = (struct user*)malloc(sizeof(struct user));
	memset (u, 0, sizeof(struct user));
	return u;
}

void url_deluser (struct user *u)
{
	if (u) {
		mfree(u->username);
		free(u);
	}
}

static void abspath (char *buf, int size)
{
	int len, i;

	len = GetModuleFileNameA (NULL, buf, size);
	for (i = len; i >=0 && buf[i]!='/' && buf[i]!='\\'; i--);
	buf[i] = '\0';
}

/* 根据WinMain参数获得uid,创建各自目录 */
int url_set_userdir (const char *param)
{
	const char *uid = param;

	if (uid==NULL || strlen(uid)==0)
		uid = "0";

	abspath(g_userdir, sizeof(g_userdir));
	abspath(g_downdir, sizeof(g_downdir));

	sprintf (g_userdir, "%s/u", g_userdir);
	mkdir (g_userdir); /* u/ */

	sprintf (g_userdir, "%s/%s", g_userdir, uid);
	mkdir (g_userdir);

	sprintf (g_downdir, "%s/download", g_userdir);
	mkdir (g_downdir);

	return !access(g_userdir, 0) && !access(g_downdir, 0);
}

char *url_get_userdir ()
{
	return g_userdir;
}

char *url_get_downdir ()
{
	return g_downdir;
}


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

	EnterCriticalSection (&g_cs);

	multi_handle = curl_multi_init();

	mkdir (g_downdir); /* 创建临时目录 */

	for (i = 0; i < g_len; i++) {
		urow *u = &g_list[i];
		char filename[256];

		sprintf (filename, "%s/%d.xml", g_downdir, u->id);

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

	LeaveCriticalSection (&g_cs);
}



static size_t
savepostresult (void *buffer, size_t size, size_t count, void *userp)
{
	int *len = (int *)userp;
	int n = size * count;

	strncpy(g_postresult + *len, buffer, n);

	*len += n;
	g_postresult[*len] = '\0';

	return n;
}


/* post登录 */
struct user *url_login (const char *username, const char *password)
{
	CURL *curl = NULL;
	CURLcode res;
	char url[128];
	int postlen = 0;

	struct curl_httppost *post = NULL;
	struct curl_httppost *last = NULL;

	struct user *user;

	EnterCriticalSection (&g_cs);

	sprintf (url, "http://service.cnal.com/?m=login&a=softlogin&rand=%u", (int)time(NULL));

	curl_formadd(&post, &last,
		CURLFORM_COPYNAME, "username",
		CURLFORM_COPYCONTENTS, username,
		CURLFORM_END);

	curl_formadd(&post, &last,
		CURLFORM_COPYNAME, "password",
		CURLFORM_COPYCONTENTS, password,
		CURLFORM_END);

	curl = curl_easy_init();
	if (curl == NULL)
		goto fail;

	curl_easy_setopt(curl, CURLOPT_HEADER, 0);
	curl_easy_setopt(curl, CURLOPT_URL, url); /*Set URL*/
	curl_easy_setopt(curl, CURLOPT_HTTPPOST, post);
	//curl_easy_setopt(curl, CURLOPT_TIMEOUT, 17);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, savepostresult);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&postlen);
	//curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
	//curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 1);

	postlen = 0;
	memset(g_postresult, 0, sizeof(g_postresult));

	res = curl_easy_perform(curl);
	if (res != CURLE_OK)
		goto fail;

	{
	char *out;
	char *s = g_postresult;
	int uid;
	char *suid, *username;

	s = nexttok(s, ',', &out);
	uid = atoi(out);
	if (uid == 0) goto fail;
	suid = dump(out);

	s = nexttok(s, ',', &out);
	username = dump(out);

	user = url_newuser();
	user->uid = uid;
	user->suid = suid;
	user->username = username;
	}

	curl_easy_cleanup(curl);
	LeaveCriticalSection (&g_cs);
	return user;

fail:
	curl_formfree(post);
	curl_easy_cleanup(curl);
	LeaveCriticalSection (&g_cs);
	return NULL;
}
