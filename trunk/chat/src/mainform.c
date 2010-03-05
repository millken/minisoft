/*
 * 主窗口|登录窗口
 */

#include "i32.h"
#include "ctrls.h"

/* main pages */
enum {
	ID_LOGPAD = 1,
	ID_LOGEDPAD
};

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

/* 登录框顶部定位 */
#define YBASE 86


/* 正式面板控件ID */
enum {
	TOPPAD = 1,
	CHATLIST,
	BOTPAD,

	AVATAR
};

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

	hpad = i32box(hwnd, "id|bc", ID_LOGPAD, 0xffffff);
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

		create_hyperlink (hpad, "id|w|t", ID_FORGOTPWD_LINK, 116, TEXT("Forgot your password?"));
		create_hyperlink (hpad, "id|w|t", ID_REGISTER_LINK, 116, TEXT("Dont't have an account?"));

	return hpad;
}


/*
 * 登录后面板
 */
static int logedpad_onsize (I32E e)
{
	i32vfill (e.hwnd,
		i32id(e.hwnd, TOPPAD), 0,
		i32id(e.hwnd, CHATLIST), -1,
		i32id(e.hwnd, BOTPAD), 0,
		NULL
	);
}

static int toppad_onsize (I32E e)
{

}

static HWND create_logedpad (HWND hwnd)
{
	HWND hpad = i32box(hwnd, "id|bc", ID_LOGEDPAD, -1);
	i32setproc (I32PRE, WM_SIZE, logedpad_onsize);

		i32box (hpad, "id|h|bc", TOPPAD, 90, 0xEE9C59);
		i32setproc (I32PRE, WM_SIZE, toppad_onsize);

			i32create(TEXT("image"), "d|s|id|w|h|cur", I32PRE, WS_CTRL, AVATAR, 40, 40, LoadCursor(NULL, IDC_HAND));
			i32send (I32PRE, IM_SETIMAGE, i32loadbmp(TEXT("AVATAR_B")), 0);
			i32send (I32PRE, IM_SETFCOLOR_HOVER, 0x555555, 0);

		i32create (TEXT("chatlist"), "n|d|s|id",
			"friendlist", hpad, WS_CTRL|WS_VSCROLL, CHATLIST);

		i32box (hpad, "id|h|bc", BOTPAD, 28, RGB(229, 239, 254));

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

	g_hwnd = hwnd = i32create(TEXT("box"), "n|s|w|h|a|bc|t",
		"mainpad", WS_OVERLAPPEDWINDOW, 270, 400, "c", -1, TEXT("Cai Shen"));
	i32setproc (I32PRE, WM_DESTROY, mainform_ondestroy);
	i32setproc (I32PRE, WM_SIZE, mainform_onsize);

	//create_logpad (hwnd);
	create_logedpad (hwnd);

	mainform_set_toppad (ID_LOGEDPAD);
	/*
	i32create(TEXT("chatlist"), "n|d|s|w|h|a",
		"friendlist", hwnd, WS_CTRL, 100, 200, "c");
	*/

	ShowWindow (hwnd, SW_SHOW);
}
