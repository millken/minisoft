#include "i32.h"
#include "ctrls.h"

#define LOGPAD 1
#define LOGINGPAD 2
#define LOGEDPAD 3

/* logpad children id */
enum {
	USERNAME_EDIT = 1,
	USERNAME_STATIC,
	PWD_EDIT,
	PWD_STATIC,
	REMPWD_CKBOX,
	REMPWD_STATIC,
	LOGO_IMG,
	LOG_BUTTON,
	FORGOTPWD_STATIC,
	REGIST_STATIC
};

/* 定位 */
#define YBASE 80

static HWND g_hwnd;
static int g_toppad = 1;

void mainform_set_toppad (int padid)
{
	g_toppad = padid;
	ShowWindow (i32id(g_hwnd,LOGPAD), SW_HIDE);
	ShowWindow (i32id(g_hwnd,LOGINGPAD), SW_HIDE);
	ShowWindow (i32id(g_hwnd,LOGEDPAD), SW_HIDE);
	ShowWindow (i32id(g_hwnd,g_toppad), SW_SHOW);
	ShowWindow (g_hwnd, SW_SHOW);
}

int mainpad_ondestroy (I32E e)
{
	PostQuitMessage(0);
	return -1;
}

int mainpad_onsize (I32E e)
{
	i32vfill (e.hwnd,
		i32id(e.hwnd, g_toppad), -1,
		NULL);
	return 0;
}

int logpad_onsize (I32E e)
{
	i32set(i32id(e.hwnd, LOGO_IMG), "a|y", "c", 20);

	i32set(i32id(e.hwnd, USERNAME_STATIC), "a|y", "c", YBASE);
	i32set(i32id(e.hwnd, USERNAME_EDIT), "a|y", "c", YBASE+18);
	i32set(i32id(e.hwnd, PWD_STATIC), "a|y", "c", YBASE+45);
	i32set(i32id(e.hwnd, PWD_EDIT), "a|y", "c", YBASE+63);
	i32set(i32id(e.hwnd, REMPWD_CKBOX), "x|y", i32clientx(i32id(e.hwnd, PWD_EDIT)), YBASE+100);
	i32set(i32id(e.hwnd, REMPWD_STATIC), "x|y", i32clientx(i32id(e.hwnd, PWD_EDIT))+16, YBASE+99);
	i32set(i32id(e.hwnd, LOG_BUTTON), "a|y", "c", YBASE+140);
	i32set(i32id(e.hwnd, FORGOTPWD_STATIC), "a|y", "c", YBASE+180);
	return 0;
}

void create_mainform ()
{
	HWND hwnd, hpad;

	g_hwnd = hwnd = i32create(TEXT("form"), "n|s|w|h|a|bc|t",
		"mainpad", WS_OVERLAPPEDWINDOW, 270, 400, "c", -1, TEXT("Cai Shen"));
	i32setproc (hwnd, WM_DESTROY, mainpad_ondestroy);
	i32setproc (hwnd, WM_SIZE, mainpad_onsize);

	/* 登录前面板 */
	hpad = i32box(hwnd, "id|bc|show", LOGPAD, 0xffffff, "y");
	i32setproc (hpad, WM_SIZE, logpad_onsize);

		i32create(TEXT("image"), "d|s|id|w|h", hpad, WS_CTRL, LOGO_IMG, 48, 48);
		i32send (I32PRE, IM_SETIMAGE, i32loadbmp("FORM_ICON"), 0);
		i32send (I32PRE, IM_SETFCOLOR_HOVER, 0xffffff, 0);
		i32send (I32PRE, IM_SETFCOLOR, 0xffffff, 0);

		i32static(hpad, "id|w|h|t|bc|fc",
			USERNAME_STATIC, 120, 18, TEXT("Username:"), -1, 0x666666);

		i32edit(hpad, "id|w|h",
			USERNAME_EDIT, 120, 20);

		i32static(hpad, "id|w|h|t|bc|fc",
			PWD_STATIC, 120, 18, TEXT("Password:"), -1, 0x666666);

		i32edit(hpad, "id|w|h",
			PWD_EDIT, 120, 20);

		i32checkbox (hpad, "id|w|t", REMPWD_CKBOX, 140, TEXT("Remember password"));
		i32setcheck (I32PRE, TRUE);

		i32create (TEXT("butten"), "d|s|id|w|h|f|t",
			hpad, WS_CTRL, LOG_BUTTON, 119, 23, "Arial,15", TEXT("Sign In"));
		i32send (I32PRE, BM_SETTEXTCOLOR, 0x000000, 0);
		i32send (I32PRE, BM_SETBCOLOR_HOVER, 0x00ffff, 0);
		i32send (I32PRE, BM_SETRADIUS, 9, 0);

		i32static (hpad, "id|w|h|f|t|fc|cur",
			FORGOTPWD_STATIC, 125, 18, "Arial,15,0,0,1", TEXT("Forgot your password?"), 0xee0000, LoadCursor(NULL,IDC_HAND));

	i32box(hwnd, "id|bc|show", LOGINGPAD, 0xff00ff, "n");
	i32box(hwnd, "id|bc|show", LOGEDPAD, 0x00ffff, "n");
	mainform_set_toppad (LOGPAD);

	/*
	i32create(TEXT("chatlist"), "n|d|s|w|h|a",
		"friendlist", hwnd, WS_CTRL, 100, 200, "c");
	*/

	ShowWindow (hwnd, SW_SHOW);
}
