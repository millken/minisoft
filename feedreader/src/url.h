#ifndef _URL_H
#define _URL_H

#ifdef __cplusplus
extern "C" {
#endif

#define DOWNLOADDIR "download"

void url_init ();
void url_close ();

int url_set_userdir (const char *param);
char *url_get_userdir ();
char *url_get_downdir ();

int url_add (int id, char *url);
void url_clear ();
void url_test ();
void url_download ();


struct user {
	int uid;
	char *suid; /* uid的字符串形式 */
	char *username;
};

struct user *url_newuser ();
void url_deluser (struct user *u);
struct user *url_login (const char *username, const char *password);

#ifdef __cplusplus
}
#endif

#endif
