/*
 * 群组对话框
 */

#include "i32.h"
#include "ctrls.h"
#include "wins.h"

enum {
	ID_TOPPAD,
	ID_MIDPAD,
	ID_BOTPAD,

	ID_TEXTVIEW,
	ID_INPUT,
	ID_VSPLIT,
	ID_MBLIST  /* member list */
};

#define PADBCOL 0xEE9C59
#define TOPPADH 40
#define BOTPADH 60
#define INPUTH 44 /* 输入框高度 */
#define INPUTMARGIN 3

static int view_onsize (I32E e)
{
	i32vfill (e.hwnd,
		i32id(e.hwnd, ID_TOPPAD), 0,
		i32id(e.hwnd, ID_MIDPAD), -1,
		i32id(e.hwnd, ID_BOTPAD), 0,
		NULL);
	return 0;
}

static int textview_onsize (I32E e)
{
	i32hfill (e.hwnd,
		i32id(e.hwnd, ID_TEXTVIEW), -1,
		i32id(e.hwnd, ID_VSPLIT), 0,
		i32id(e.hwnd, ID_MBLIST), 0,
		NULL);
	return 0;
}

static int botpad_onsize (I32E e)
{
	HWND hinput = i32id(e.hwnd, ID_INPUT);
	RECT r;
	int h = i32clienth(hinput);

	GetClientRect(e.hwnd, &r);
	i32set (hinput, "x|y|w",
		INPUTMARGIN, (r.bottom-h)/2, r.right-INPUTMARGIN*2);
	return 0;
}

/* 发出消息 */
static int input_onkeydown (I32E e)
{
	HWND hview;
	TCHAR buf[1024];

	/* CTRL+回车 */
	if (e.wp==VK_RETURN && (GetKeyState(VK_CONTROL)&0x8000)) {
		richedit_gettext (e.hwnd, buf);
		hview = GetParent(GetParent(e.hwnd));
		hview = i32id(i32id(hview, ID_MIDPAD), ID_TEXTVIEW);
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
	if (e.wp == ID_TEXTVIEW) {
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

HWND create_groupview (int gid)
{
	HWND hwnd, hpad;
	HWND hinput;

	if (gid <= 0) return NULL;

	hwnd = create_form (TEXT("多人讨论 - 汪汪汪"), "n|w|h|a|bc",
		"g001", 330, 330, "c", -1);
	i32setproc (hwnd, WM_SIZE, view_onsize);

	/* TOP */
	hpad = i32box(hwnd, "id|bc|h", ID_TOPPAD, PADBCOL, TOPPADH);

	/* MID */
	hpad = i32box(hwnd, "id|bc", ID_MIDPAD, 0xFFFFFF);
	i32setproc (hpad, WM_SIZE, textview_onsize);
	i32setproc (hpad, WM_NOTIFY, midpad_onnotify);

		/* text view */
		create_richedit (hpad, "id|+s|w|h", ID_TEXTVIEW, WS_VSCROLL|ES_READONLY, 100, 100);
		richedit_autolink (I32PRE, TRUE);
		richedit_setfont (I32PRE, TRUE, "facename|size|bold|color", TEXT("Arial"), 9, FALSE, 0x000000, TRUE);
		richedit_textout (I32PRE, TEXT("Go! http://cnal.com\n haha_\n"));

		/* split */
		i32box (hpad, "id|w|bc", ID_VSPLIT, 2, PADBCOL);

		/* member list */
		create_cl(hpad, "id|w", ID_MBLIST, 130);
		cl_viewmode (I32PRE, 1);

	/* BOT */
	hpad = i32box(hwnd, "id|bc|h", ID_BOTPAD, PADBCOL, BOTPADH);
	i32setproc (I32PRE, WM_SIZE, botpad_onsize);

		hinput = create_richedit(hpad, "id|+s|h", ID_INPUT, WS_BORDER, INPUTH);
		richedit_setfont (I32PRE, TRUE, "facename|size|bold|color|offset", TEXT("Verdana"), 9, FALSE, 0x000000, -50);
		i32setproc (I32PRE, WM_KEYDOWN, input_onkeydown);


	ShowWindow (hwnd, SW_SHOW);
	SetFocus(hinput);
	return hwnd;
}
