#include "i32.h"

void reg_form ();
void reg_chatlist ();

void reg_myctl ()
{
	reg_form();
	reg_chatlist();
}

int mainform_onclose (I32E e)
{
	PostQuitMessage(0);

	return 0;
}

int bigform_onsize (I32E e)
{
	i32vfill ( e.hwnd,
		i32("panel-t"), 50,
		i32("friendlist"), -1,
		i32("panel-b"), 40,
		NULL
	);
	return 0;
}

void create_form ()
{
	reg_myctl ();

	HWND hwnd = i32create (TEXT("form"), "n|t|s|w|h|a|bc", "bigform", TEXT("財神說"),
				WS_OVERLAPPEDWINDOW
				, 270, 500, "c", -1);
	i32setproc (hwnd, WM_DESTROY, mainform_onclose);
	i32setproc (hwnd, WM_SIZE, bigform_onsize);

	i32box ("panel-t", hwnd);
	i32set (i32("panel-t"), "bc", 0xEE9C59);

	i32box ("panel-b", hwnd);
	i32set (i32("panel-b"), "bc", RGB(89,156,238));

	i32create (TEXT("chatlist"), "d|n|s|w|h|a|show", hwnd, "friendlist",
				WS_CTRL|WS_VSCROLL, 100, 100, "c", "y");

	ShowWindow (hwnd, SW_SHOW);
}

int setbox_onsize (I32E e)
{
	i32vfill (e.hwnd,
		i32("dlg-top"), 40,
		i32("viewer"), -1,
		i32("dlg-foot"), 53,
		NULL
		);

	return 0;
}

int dlgfoot_onsize (I32E e)
{
	i32set (i32("inputer"), "w|h|a|+y", i32clientw(e.hwnd)-8, i32clienth(e.hwnd)-32,
		"c", 8);
	return 0;
}

int setbox_onclose (I32E e)
{
	SetFocus(i32("bigform"));
	return -1;
}

int WINAPI WinMain (HINSTANCE hithis, HINSTANCE hiold, PSTR param, int cmd)
{
	reg_myctl ();

	create_form ();

	HWND hwnd = i32create(TEXT("form"), "n|t|s|w|h|a|+x|+y|bc", "setbox",
			TEXT("对话 Dog"), WS_OVERLAPPEDWINDOW, 350, 350,
			"c", 100, 50, -1);
	i32setproc (hwnd, WM_SIZE, setbox_onsize);
	i32setproc (hwnd, WM_CLOSE, setbox_onclose);

	i32box ("dlg-top", hwnd);
	i32set (i32("dlg-top"), "bc", RGB(89,156,238));
	i32create (TEXT("edit"), "n|d|s|t", "viewer", hwnd,
			WS_CTRL|ES_MULTILINE|ES_AUTOHSCROLL | ES_AUTOVSCROLL,
			TEXT("似的")
			);
	i32box ("dlg-foot", hwnd);
	i32set (i32("dlg-foot"), "bc", RGB(89,156,238));
	i32create (TEXT("edit"), "n|d|s|t|h", "inputer", i32("dlg-foot"),
			WS_CTRL|ES_MULTILINE|ES_AUTOHSCROLL | ES_AUTOVSCROLL|WS_BORDER,
			TEXT(""), 22
			);
	i32setproc (i32("dlg-foot"), WM_SIZE, dlgfoot_onsize);

	ShowWindow (hwnd, SW_SHOW);

	i32loop();
	return 0;
}
