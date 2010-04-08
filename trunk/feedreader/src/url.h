#ifndef _URL_H
#define _URL_H

#ifdef __cplusplus
extern "C" {
#endif

#define DOWNLOADDIR "download"

int url_set_userdir (char *param);
char *url_get_userdir ();
char *url_get_downdir ();

int url_add (int id, char *url);
void url_clear ();
void url_test ();
void url_download ();

#ifdef __cplusplus
}
#endif

#endif
