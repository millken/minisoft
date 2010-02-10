#ifndef _CTRLS_H
#define _CTRLS_H

#include <windows.h>
#include <richedit.h>

/* 好友列表控件的消息 */
enum CHATLIST_MSG {

	CM_SETVIEWMODE = WM_USER+1,
	CM_GETVIEWMODE,
	CM_ADDGROUP,
	CM_SETGROUP_NAME,
	CM_SETGROUP_NOTE,
	CM_DELGROUP,

	CM_ADDBUDDY,
	CM_SETBUDDY_NAME,
	CM_SETBUDDY_NOTE,
	CM_SETBUDDY_SIGN,
	CM_SETBUDDY_PIC,
	CM_SETBUDDY_STATUS,
	CM_DELBUDDY,

	CM_BLSORT, /* 单组好友列表重新排序 */

	/* 获得当前鼠标所在的元素,
	   wp=0:无, <0:组GID, >0:好友UID; */
	CM_GETSELECT,

	/* 反馈 */
	CM_LDOWN, /* 左键按下 */
	CM_LUP,
	CM_RDOWN, /* 右键按下 */
	CM_RUP
};

/* 按钮控件消息 */
enum {
	BM_SETRADIUS = WM_USER + 1,
	BM_SETALIGN,
	BM_SETICON,
	BM_SETICONALIGN,
	BM_SETMARGIN,
	BM_SETTITLEMARGIN,
	BM_SETBCOLOR,
	BM_SETBCOLOR_HOVER,
	BM_SETBCOLOR_PUSHED,
	BM_SETFCOLOR,
	BM_SETTEXTCOLOR,

	/* 输出消息 */
	BM_LBUTTONDOWN,
	BM_LBUTTONUP,
	BM_RBUTTONDOWN,
	BM_RBUTTONUP
};

/* 图片控件消息 */
enum {
	IM_SETIMAGE = WM_USER + 1,
	IM_SETPADDING,
	IM_SETBCOLOR,
	IM_SETBCOLOR_HOVER,
	IM_SETFCOLOR,
	IM_SETFCOLOR_HOVER
};

/* richedit控件 */
#define RICHEDIT_STYLE (WS_VISIBLE | WS_CHILD | WS_TABSTOP | WS_VSCROLL | ENM_LINK)
HWND new_richedit (HWND dad, char *format, ...);
void richedit_setfont (HWND hwnd, BOOL isall, char *format, ...);
void richedit_textout (HWND hrich, TCHAR *text);
void richedit_autolink (HWND hrich, BOOL isauto);
void richedit_clear (HWND hrich);
void richedit_gettext (HWND hrich, TCHAR *buf);

#endif
