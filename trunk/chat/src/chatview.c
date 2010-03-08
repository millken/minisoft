/*
 * 单人对话框
 */
#include "i32.h"
#include "ctrls.h"
#include "wins.h"

#define PADBCOLOR 0xEE9C59
#define INPUT_PADDING 3

/* CHATVIEW ID */
enum {
	ID_TOPPAD,
	ID_MIDPAD,
	ID_BOTPAD,

	ID_VIEW,
	ID_INPUT
};

static int chatview_ondestroy (I32E e)
{
	PostQuitMessage(0);
	return 0;
}

static int chatview_onsize (I32E e)
{
	i32vfill (e.hwnd,
		i32id(e.hwnd, ID_TOPPAD), 0,
		i32id(e.hwnd, ID_MIDPAD), -1,
		i32id(e.hwnd, ID_BOTPAD), 0,
		NULL
		);
	return 0;
}

static int midpad_onsize (I32E e)
{
	i32vfill (e.hwnd,
		i32id(e.hwnd, ID_VIEW), -1,
		NULL);
	return 0;
}

static int footpad_onsize (I32E e)
{
	RECT r;
	HWND hinput;
	int h;

	hinput = i32id(e.hwnd, ID_INPUT);
	h = i32clienth(hinput);
	GetClientRect(e.hwnd, &r);
	i32set (i32id(e.hwnd, ID_INPUT), "x|y|w",
		INPUT_PADDING, (r.bottom-h)/2, r.right-INPUT_PADDING*2);
	return 0;
}

/* 发出消息 */
static int input_onkeydown (I32E e)
{
	HWND hview;
	TCHAR buf[1024];

	if (e.wp==VK_RETURN && (GetKeyState(VK_CONTROL)&0x8000)) {
		richedit_gettext (e.hwnd, buf);
		hview = GetParent(GetParent(e.hwnd));
		hview = i32id(i32id(hview, ID_MIDPAD), ID_VIEW);
		richedit_textout (hview, buf);
		richedit_textout (hview, TEXT("\n"));
		i32send (hview, WM_VSCROLL, SB_BOTTOM, 0);
		richedit_clear (e.hwnd);
		return 0;
	}

	return -1;
}

/* 点链接 */
static int midpad_onnotify (I32E e)
{
	if (e.wp == ID_VIEW) {
		ENLINK *el = (ENLINK *)e.lp;
		if (el && el->nmhdr.code == EN_LINK) {
			HWND hrich = el->nmhdr.hwndFrom;
			if (el->msg == WM_LBUTTONDOWN) {
				TCHAR buf[320] = {0};
				int len;
				i32send (hrich, EM_EXSETSEL, 0, &el->chrg);
				len = i32send (hrich, EM_GETSELTEXT, 0, buf);
				if (len > 0)
					ShellExecute (NULL, TEXT("open"), buf, NULL, NULL, SW_SHOW);
			}
		}

	}

	return 0;
}

HWND create_chatview (int uid)
{
	HWND hwnd, hpad;
	char buf[32];

	sprintf (buf, "u%d", uid);

	hwnd = i32create(TEXT("form"), "n|s|w|h|a|t|bc",
		buf, WS_OVERLAPPEDWINDOW, 400, 240, "c", TEXT("对话 - 汪汪"), -1);
	i32setproc (hwnd, WM_DESTROY, chatview_ondestroy);
	i32setproc (hwnd, WM_SIZE, chatview_onsize);

	hpad = i32box (hwnd, "id|bc|h", ID_TOPPAD, PADBCOLOR, 40);

	hpad = i32box (hwnd, "id|bc", ID_MIDPAD, 0xFFFFFF);
	i32setproc (hpad, WM_SIZE, midpad_onsize);
	i32setproc (hpad, WM_NOTIFY, midpad_onnotify);

		create_richedit (hpad, "id|+s|w|h", ID_VIEW, WS_VSCROLL|ES_READONLY, 100, 100);
		richedit_autolink (I32PRE, TRUE);
		richedit_setfont (I32PRE, TRUE, "facename|size|bold|color", TEXT("Arial"), 9, FALSE, 0x000000, TRUE);
		richedit_textout (I32PRE, TEXT("Go! http://cnal.com\n haha_\n"));

	hpad = i32box (hwnd, "id|bc|h", ID_BOTPAD, PADBCOLOR, 60);
	i32setproc (hpad, WM_SIZE, footpad_onsize);

		create_richedit(hpad, "id|+s|h", ID_INPUT, WS_BORDER, 44);
		richedit_setfont (I32PRE, TRUE, "facename|size|bold|color|offset", TEXT("Verdana"), 9, FALSE, 0x000000, -50);
		i32setproc (I32PRE, WM_KEYDOWN, input_onkeydown);

	ShowWindow(hwnd, SW_SHOW);
	SetFocus(i32id(i32id(hwnd, ID_BOTPAD), ID_INPUT));

	return hwnd;
}
