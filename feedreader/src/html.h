#ifndef _HTML_H
#define _HTML_H

#ifdef __cplusplus
extern "C" {
#endif

void reg_html_control ();
HWND html_create (HWND dad, char *format, ...);
int html_loadfile (HWND hwnd, wchar_t *filename);
int html_loadstring (HWND hwnd, char *html);

#ifdef __cplusplus
}
#endif

#endif
