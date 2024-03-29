/*
 * 主窗口|登录窗口
 */

#include "i32.h"
#include "ctrls.h"
#include "wins.h"

/* 登录面板控件ID */
enum {
	ID_USERNAME_EDIT = 1,
	ID_USERNAME_STATIC,
	ID_PWD_EDIT,
	ID_PWD_STATIC,
	ID_REMPWD_CKBOX,
	ID_REMPWD_STATIC,
	ID_LOGO_IMG,
	ID_LOG_BUTTON,
	ID_FORGOTPWD_LINK,
	ID_REGISTER_LINK
};

/* 正式面板控件ID */
enum {
	ID_TOPPAD = 1,
	ID_BOTPAD,
	ID_CHATLIST,
	ID_GROUPLIST,
	ID_CONTACTLIST,
	ID_APPLIST,

	ID_AVATAR, /* 头像框 */
	ID_NAME,
	ID_SIGN,  /* 签名框 */
	ID_SEARCHBOX, /* 搜索框 */
	ID_TAB_FRIEND,
	ID_TAB_GROUP,
	ID_TAB_CONTACT,
	ID_TAB_APP
};

/* 登录框顶部定位 */
#define YBASE 86

static HWND g_hwnd;
static int g_toppad = 1;

void mainform_set_toppad (int padid)
{
	HWND nowhwnd;
	g_toppad = padid;
	nowhwnd = i32id(g_hwnd,g_toppad);

	ShowWindow (i32id(g_hwnd,ID_LOGPAD), SW_HIDE);
	ShowWindow (i32id(g_hwnd,ID_LOGEDPAD), SW_HIDE);

	SendMessage(g_hwnd, WM_SIZE, 0, 0);
	SetWindowPos (nowhwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW);
}


/*
 * 登录面板
 */

/* 登陆窗口布局 */
static int logpad_onsize (I32E e)
{
	i32set(i32id(e.hwnd, ID_LOGO_IMG), "a|y", "c", 20);

	i32set(i32id(e.hwnd, ID_USERNAME_STATIC), "a|y", "c", YBASE);
	i32set(i32id(e.hwnd, ID_USERNAME_EDIT), "a|y", "c", YBASE+18);
	i32set(i32id(e.hwnd, ID_PWD_STATIC), "a|y", "c", YBASE+45);
	i32set(i32id(e.hwnd, ID_PWD_EDIT), "a|y", "c", YBASE+63);
	i32set(i32id(e.hwnd, ID_REMPWD_CKBOX), "x|y", i32clientx(i32id(e.hwnd, ID_PWD_EDIT)), YBASE+100);
	i32set(i32id(e.hwnd, ID_REMPWD_STATIC), "x|y", i32clientx(i32id(e.hwnd, ID_PWD_EDIT))+16, YBASE+99);
	i32set(i32id(e.hwnd, ID_LOG_BUTTON), "a|y", "c", YBASE+140);
	i32set(i32id(e.hwnd, ID_FORGOTPWD_LINK), "a|y", "c", YBASE+180);
	i32set(i32id(e.hwnd, ID_REGISTER_LINK), "a|y", "c", YBASE+200);
	return 0;
}

static int logbutton_onlbup (I32E e)
{
	EnableWindow(e.hwnd, FALSE);
	EnableWindow(i32("unamebox"), FALSE);
	EnableWindow(i32("pwdbox"), FALSE);

	mainform_set_toppad (ID_LOGEDPAD);
	return -1;
}

static HWND create_logpad (HWND hwnd)
{
	HWND hpad;

	hpad = i32box(hwnd, "n|id|bc", "logpad", ID_LOGPAD, 0xffffff);
	i32setproc (hpad, WM_SIZE, logpad_onsize);

		i32create(TEXT("image"), "d|s|id|w|h", hpad, WS_CTRL, ID_LOGO_IMG, 48, 48);
		i32send (I32PRE, IM_SETIMAGE, i32loadbmp(TEXT("FORM_ICON")), 0);
		i32send (I32PRE, IM_SETFCOLOR_HOVER, 0xffffff, 0);
		i32send (I32PRE, IM_SETFCOLOR, 0xffffff, 0);

		i32static(hpad, "id|w|h|t|bc|fc",
			ID_USERNAME_STATIC, 140, 18, TEXT("Username:"), -1, 0x666666);

		i32edit(hpad, "n|id|w|h",
			"unamebox", ID_USERNAME_EDIT, 140, 20);

		i32static(hpad, "id|w|h|t|bc|fc",
			ID_PWD_STATIC, 140, 18, TEXT("Password:"), -1, 0x666666);

		i32pwdedit(hpad, "n|id|w|h",
			"pwdbox", ID_PWD_EDIT, 140, 20);

		i32checkbox (hpad, "id|w|t", ID_REMPWD_CKBOX, 140, TEXT("Remember password"));
		i32setcheck (I32PRE, TRUE);

		create_butten (hpad, TEXT("LONGBUTTON"), "n|id|t", "logbtn", ID_LOG_BUTTON, TEXT("Sign In"));
		i32setproc (I32PRE, WM_LBUTTONUP, logbutton_onlbup);

		create_link (hpad, "id|w|t", ID_FORGOTPWD_LINK, 116, TEXT("Forgot your password?"));
		create_link (hpad, "id|w|t", ID_REGISTER_LINK, 116, TEXT("Dont't have an account?"));

	return hpad;
}


/*
 * 登录后面板
 */

#define TABBCOL 0xEE9C59 //RGB(229, 239, 254)
#define TABNCOL 0xF8D2B4 //0xFFFFFF

char *g_pagename = "friendlist";

static int logedpad_onsize (I32E e)
{
	i32vfill (e.hwnd,
		i32id(e.hwnd, ID_TOPPAD), 0,
		i32(g_pagename), -1,
		i32id(e.hwnd, ID_BOTPAD), 0,
		NULL
	);
	return 0;
}

static int toppad_onsize (I32E e)
{
	int w = i32clientw(e.hwnd);

	i32set(i32id(e.hwnd, ID_AVATAR), "a|-x|+y", "t|r", 7, 11);
	i32set(i32id(e.hwnd, ID_NAME), "x|y", 11, 10);
	i32set(i32id(e.hwnd, ID_SIGN), "x|y", 6, 33);
	i32set(i32id(e.hwnd, ID_SEARCHBOX), "x|y|w", 8, 62, w-16);

	return 0;
}

static int footpad_onsize (I32E e)
{
	i32hfill (e.hwnd,
		i32id(e.hwnd, ID_TAB_FRIEND), 0,
		i32id(e.hwnd, ID_TAB_GROUP), 0,
		i32id(e.hwnd, ID_TAB_CONTACT), 0,
		i32id(e.hwnd, ID_TAB_APP), 0,
		NULL);
	return 0;
}

static void set_listpage (char *pagename)
{
	if (!pagename) return;

	g_pagename = pagename;

	ShowWindow (i32("chatlist"), SW_HIDE);
	ShowWindow (i32("grouplist"), SW_HIDE);
	ShowWindow (i32("contactlist"), SW_HIDE);
	ShowWindow (i32("applist"), SW_HIDE);

	ShowWindow (i32(pagename), SW_SHOW);
	i32send (i32("logedpad"), WM_SIZE, 0, 0);
}

static void set_tab (int id)
{
	char *pagename = NULL;
	HWND botpad = i32id(i32("logedpad"), ID_BOTPAD);

	i32send(i32id(botpad, ID_TAB_FRIEND), BM_SETBCOLOR, TABBCOL, 0);
	i32send(i32id(botpad, ID_TAB_GROUP), BM_SETBCOLOR, TABBCOL, 0);
	i32send(i32id(botpad, ID_TAB_CONTACT), BM_SETBCOLOR, TABBCOL, 0);
	i32send(i32id(botpad, ID_TAB_APP), BM_SETBCOLOR, TABBCOL, 0);

	i32send(i32id(botpad, id), BM_SETBCOLOR, TABNCOL, 0);

	switch (id) {
		case ID_TAB_FRIEND: pagename = "chatlist"; break;
		case ID_TAB_GROUP: pagename = "grouplist"; break;
		case ID_TAB_CONTACT: pagename = "contactlist"; break;
		case ID_TAB_APP: pagename = "applist"; break;
	}

	set_listpage (pagename);

	i32send (botpad, WM_SIZE, 0, 0);
	InvalidateRect (botpad, NULL, TRUE);
}

static int footpad_oncommand (I32E e)
{
	int id = LOWORD(e.wp);

	if (HIWORD(e.wp) == BM_LBUTTONUP)
		set_tab(id);

	return 0;
}

/* 打开对话框 */
static int logedpad_oncommand (I32E e)
{
	HWND hwnd = (HWND)e.lp;
	int uid, gid;

	if (HIWORD(e.wp) == CM_LUP) {
		switch (LOWORD(e.wp)) {
			case ID_CHATLIST: {
				uid = cl_getsel(hwnd);
				create_chatview (uid);
			}
			break;

			case ID_GROUPLIST: {
				gid = cl_getsel(hwnd);
				create_groupview (gid);
			}
			break;
		}
	}
	return 0;
}

static HWND create_logedpad (HWND hwnd)
{
	HWND hpad, hdiv;

	hpad = i32box(hwnd, "n|id|bc", "logedpad", ID_LOGEDPAD, -1);
	i32setproc (I32PRE, WM_SIZE, logedpad_onsize);
	i32setproc (I32PRE, WM_COMMAND, logedpad_oncommand); /* 接收列表事件 */

		/* 头部 */
		hdiv = i32box (hpad, "id|h|bc", ID_TOPPAD, 90, 0xEE9C59);
		i32setproc (I32PRE, WM_SIZE, toppad_onsize);

			/* 头像 */
			create_image (hdiv, TEXT("AVATAR_B"), "id|w|h|cur",
				ID_AVATAR, 40, 40, LoadCursor(NULL, IDC_HAND));
			i32send (I32PRE, IM_SETFCOLOR_HOVER, 0x222222, 0);
			i32send (I32PRE, IM_SETPADDING, 4, 0);

			/* 名字框 */
			create_link (hdiv, "id|w|f|t", ID_NAME, 120, "Arial,15,0", TEXT("Cat street"));
			link_setcolor (I32PRE, 0, 0x00ccff, 0x00ccff);

			/* 签名框 */
			i32create(TEXT("butten"), "d|s|id|t|w|h|a|f|show",
				hdiv, WS_CTRL, ID_SIGN, TEXT("지 못한哈哈Эучены"),
						140, 22, "c|t", "Verdana,15", "y");
			i32send (I32PRE, BM_SETBCOLOR, RGB(89,156,238), 0);
			i32send (I32PRE, BM_SETFCOLOR, 0xeeeeee, 0);
			i32send (I32PRE, BM_SETBCOLOR_HOVER, RGB(160,199,245), 0);
			i32send (I32PRE, BM_SETBCOLOR_PUSHED, RGB(160,199,245), 0);
			i32send (I32PRE, BM_SETRADIUS, 2, 0);
			i32send (I32PRE, BM_SETMARGIN, 5, 0);
			i32send (I32PRE, BM_SETTEXTCOLOR, 0xeeeeee, 0);
			i32send (I32PRE, BM_SETALIGN, 0, 0);

			/* 搜索框 */
			hwnd = create_richedit (hdiv, "+s|id|h|f", WS_BORDER, ID_SEARCHBOX, 22, "Verdana,15,0");
			richedit_setfont (hwnd, TRUE, "color|offset", 0x000000, -23);


		/* 中部 */
		/* 好友列表 */
		create_cl (hpad, "n|id", "chatlist", ID_CHATLIST);
		/* 群组列表 */
		create_cl (hpad, "n|id", "grouplist", ID_GROUPLIST);
		cl_viewmode (I32PRE, 1);
		/* 最近联系人列表 */
		create_cl (hpad, "n|id", "contactlist", ID_CONTACTLIST);
		/* 应用列表 */
		i32box (hpad, "n|id|bc", "applist", ID_APPLIST, 0x00ccff);

		/* 底部 */
		hdiv = i32box (hpad, "id|h|bc", ID_BOTPAD, 28, TABBCOL);
		i32setproc (I32PRE, WM_SIZE, footpad_onsize);
		i32setproc (I32PRE, WM_COMMAND, footpad_oncommand);
		{
			HWND htab[4];
			int i;
			/* tabs */
			htab[0] = i32create(TEXT("butten"), "d|s|id|t|w|h|f", hdiv, WS_CTRL, ID_TAB_FRIEND, TEXT("好友"), 44, 20, "Verdana,15,1");
			htab[1] = i32create(TEXT("butten"), "d|s|id|t|w|h|f", hdiv, WS_CTRL, ID_TAB_GROUP, TEXT("群组"), 44, 20, "Verdana,15,1");
			htab[2] = i32create(TEXT("butten"), "d|s|id|t|w|h|f", hdiv, WS_CTRL, ID_TAB_CONTACT, TEXT("最近联系人"), 80, 20, "Verdana,15,1");
			htab[3] = i32create(TEXT("butten"), "d|s|id|t|w|h|f", hdiv, WS_CTRL, ID_TAB_APP, TEXT("应用"), 40, 20, "Verdana,15,1");
			for (i = 0; i < 4; i++) {
				SendMessage (htab[i], BM_SETBCOLOR, TABBCOL, 0);
				SendMessage (htab[i], BM_SETBCOLOR_HOVER, TABNCOL, 0);
				SendMessage (htab[i], BM_SETBCOLOR_PUSHED, TABNCOL, 0);
				SendMessage (htab[i], BM_SETFCOLOR, TABNCOL, 0);
			}
		}

	set_tab (ID_TAB_GROUP);
	return hpad;
}


/*
 * 主窗口
 */

/* 退出 */
static int mainform_ondestroy (I32E e)
{
	PostQuitMessage(0);
	return -1;
}

static int mainform_onsize (I32E e)
{
	i32vfill (e.hwnd,
		i32id(e.hwnd, g_toppad), -1,
		NULL);
	return 0;
}

void create_mainform ()
{
	HWND hwnd;

	g_hwnd = hwnd = create_form (TEXT("Cai Shen"), "n|w|h|a|bc",
		"mainform", 270, 400, "c", -1);
	i32setproc (I32PRE, WM_DESTROY, mainform_ondestroy);
	i32setproc (I32PRE, WM_SIZE, mainform_onsize);

	create_logpad (hwnd);
	create_logedpad (hwnd);

	mainform_set_toppad (ID_LOGEDPAD);

	ShowWindow (hwnd, SW_SHOW);
}
