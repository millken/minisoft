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
	LOGO_IMG
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
	i32set(i32id(e.hwnd, LOGO_IMG), "a|y", "c", 10);

	i32set(i32id(e.hwnd, USERNAME_STATIC), "a|y", "c", YBASE);
	i32set(i32id(e.hwnd, USERNAME_EDIT), "a|y", "c", YBASE+18);
	i32set(i32id(e.hwnd, PWD_STATIC), "a|y", "c", YBASE+45);
	i32set(i32id(e.hwnd, PWD_EDIT), "a|y", "c", YBASE+63);
	i32set(i32id(e.hwnd, REMPWD_CKBOX), "x|y", i32clientx(i32id(e.hwnd, PWD_EDIT)), YBASE+100);
	i32set(i32id(e.hwnd, REMPWD_STATIC), "x|y", i32clientx(i32id(e.hwnd, PWD_EDIT))+16, YBASE+99);
	return 0;
}

void create_mainform ()
{
	HWND hwnd, hpad, hlogo;

	g_hwnd = hwnd = i32create(TEXT("box"), "n|s|w|h|a|bc",
		"mainpad", WS_OVERLAPPEDWINDOW, 270, 400, "c", -1);
	i32setproc (hwnd, WM_DESTROY, mainpad_ondestroy);
	i32setproc (hwnd, WM_SIZE, mainpad_onsize);

	/* 登录前面板 */
	hpad = i32box(hwnd, "id|bc|show", LOGPAD, 0xffffff, "y");
	i32setproc (hpad, WM_SIZE, logpad_onsize);
	/* ****************** */
	hlogo = i32create(TEXT("image"), "d|s|id|w|h", hpad, WS_CTRL, LOGO_IMG, 48, 48);
	i32sendmsg (hlogo, IM_SETIMAGE, i32loadbmp("FORM_ICON"), 0);
	i32sendmsg (hlogo, IM_SETFCOLOR_HOVER, 0xffffff, 0);
	i32sendmsg (hlogo, IM_SETFCOLOR, 0xffffff, 0);

	i32static(hpad, "id|w|h|t",
		USERNAME_STATIC, 100, 20, TEXT("Username:"));
	i32edit(hpad, "id|w|h",
		USERNAME_EDIT, 100, 21);
	i32static(hpad, "id|w|h|t",
		PWD_STATIC, 100, 20, TEXT("Password:"));
	i32edit(hpad, "id|w|h",
		PWD_EDIT, 100, 21);
	i32checkbox (hpad, "id", REMPWD_CKBOX);
	i32static (hpad, "id|w|h|t", REMPWD_STATIC, 120, 23, TEXT("Remmber password"));

	i32box(hwnd, "id|bc|show", LOGINGPAD, 0xff00ff, "n");
	i32box(hwnd, "id|bc|show", LOGEDPAD, 0x00ffff, "n");
	mainform_set_toppad (LOGPAD);

	/*
	i32create(TEXT("chatlist"), "n|d|s|w|h|a",
		"friendlist", hwnd, WS_CTRL, 100, 200, "c");
	*/
	ShowWindow (hwnd, SW_SHOW);
}
