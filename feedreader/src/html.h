#ifndef _HTML_H
#define _HTML_H

#include "xml.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*htmlfeedevt)(HWND hhtml, int feedid); /* 点feed事件 */
typedef void (*html_subcb) (HWND hwnd, char *url); /* 订阅按钮事件 */

void reg_html_control ();
HWND html_create (HWND dad, const char *format, ...);

int html_loadfile (HWND hwnd, const wchar_t *filename);
int html_loadstring (HWND hwnd, char *html);

/* 刷新 */
void html_updatewindow (HWND hwnd);

/* 加载feedlist到左侧 */
void html_loadfeedlist (HWND hwnd);
void html_clearfeedlist (HWND hwnd);
void html_refresh_feedlist (HWND hwnd);
void html_insertfeed (HWND hwnd, Feed *feed);

void html_insertitem (HWND hwnd, FeedItem *item);
void html_loaditemlist (HWND hwnd, int feedid);
void html_clearitemlist (HWND hwnd);

/* 设置标题 */
void html_setboard (HWND hwnd, const char *feedtitle);
/* 设置订阅按钮回调 */
void html_set_subevt (HWND hwnd, html_subcb f);

void html_showtip(HWND hwnd, const char *info, int state);
void html_hidetip(HWND hwnd);

/* 左侧菜单是否弹出 */
BOOL html_is_onleft ();


#ifdef __cplusplus
}
#endif

#endif
