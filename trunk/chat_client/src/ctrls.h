/*
 * 各自独立的自定义控件,相当于模型
 */
#ifndef _CTRLS_H
#define _CTRLS_H

#include <windows.h>
#include <richedit.h>

/* 全局对齐参数 */
enum {
	LEFT = 0,
	RIGHT,
	CENTER
};

/* 皮肤窗口控件 */
HWND create_form (TCHAR *title, char *format, ...);


/* 联系人列表控件的消息 */
enum CHATLIST_MSG {

	CM_SETVIEWMODE = WM_USER,
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
HWND create_cl (HWND dad, char *format, ...);
void cl_viewmode (HWND hwnd, int mode); /* 0:显示大头像, 1:小头像, 2:无头像 */
int cl_getsel (HWND hwnd);

/* 按钮控件消息 */
enum {
	BM_SETRADIUS = WM_USER,  /* 半径: int wp */
	BM_SETALIGN,  /* 对齐: const wp={LEFT,RIGHT,CENTER} */
	BM_SETICON,  /* 图标: HBITMAP wp */
	BM_SETICONALIGN,  /* 图标对齐: const wp={LEFT,RIGHT} */
	BM_SETMARGIN,  /* 左右边距: int wp */
	BM_SETTITLEMARGIN,  /* 文字边距: int wp */
	BM_SETBCOLOR,  /* 背景色: DWORD wp */
	BM_SETBCOLOR_HOVER,
	BM_SETBCOLOR_PUSHED,
	BM_SETFCOLOR,  /* 边框色: DWORD wp */
	BM_SETTEXTCOLOR,  /* 文字色: DWORD wp */
	BM_SETIMG, /* 状态图片, 从上到下纵排4个状态: 正常,经过,按下,禁用 */

	/* 反馈给父窗口的消息 */
	/* WM_COMMAND HIWORD=下面的消息 LOWORD=控件id */
	BM_LBUTTONDOWN,
	BM_LBUTTONUP,
	BM_RBUTTONDOWN,
	BM_RBUTTONUP
};
HWND create_butten (HWND dad, TCHAR *imagercname, char *format, ...);



/* 超链接控件 */
enum {
	LM_SETCOLOR = WM_USER, /* DWORD wp = 默认文字颜色 */
	LM_SETCOLOR_HOVER,
	LM_SETCOLOR_PUSH
};
HWND create_link (HWND dad, char *format, ...);
void link_setcolor (HWND hwnd, DWORD color, DWORD colorh, DWORD colorp);


/* 图片控件消息 */
enum {
	IM_SETIMAGE = WM_USER,
	IM_SETPADDING,
	IM_SETBCOLOR,
	IM_SETBCOLOR_HOVER,
	IM_SETFCOLOR,
	IM_SETFCOLOR_HOVER
};
HWND create_image (HWND dad, TCHAR *rcname, char *format, ...);



/* richedit控件 */
#define RICHEDIT_STYLE (WS_VISIBLE | WS_CHILD | WS_TABSTOP | WS_VSCROLL | ENM_LINK)
HWND create_richedit (HWND dad, char *format, ...);
void richedit_setfont (HWND hwnd, BOOL isall, char *format, ...);
void richedit_textout (HWND hrich, TCHAR *text);
void richedit_autolink (HWND hrich, BOOL isauto);
void richedit_clear (HWND hrich);
void richedit_gettext (HWND hrich, TCHAR *buf);



/* 注册所有控件 */
void reg_form();
void reg_chatlist();
void reg_butten();
void reg_image();
void reg_hyperlink();
#define reg_ctrls() { \
	reg_form(); \
	reg_chatlist(); \
	reg_butten(); \
	reg_image();\
	reg_hyperlink();\
	}


#endif
