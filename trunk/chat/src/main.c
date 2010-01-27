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

	HWND hwnd = i32create (TEXT("form"), "n|t|s|w|h|a|bc", "bigform", TEXT("財神"),
				WS_OVERLAPPEDWINDOW
				, 270, 500, "c", -1);
	i32setproc (hwnd, WM_DESTROY, mainform_onclose);
	i32setproc (hwnd, WM_SIZE, bigform_onsize);

	i32box ("panel-t", hwnd);
	i32set (i32("panel-t"), "bc", RGB(89,156,238));

	i32box ("panel-b", hwnd);
	i32set (i32("panel-b"), "bc", RGB(89,156,238));

	i32create (TEXT("chatlist"), "d|n|s|w|h|a|show", hwnd, "friendlist",
				WS_CTRL|WS_VSCROLL, 100, 100, "c", "y");

	ShowWindow (hwnd, SW_SHOW);
}

int setbox_onsize (I32E e)
{
	i32vfill (e.hwnd,
		i32("dlg-top"), 30,
		NULL
		);

	return 0;
}

int WINAPI WinMain (HINSTANCE hithis, HINSTANCE hiold, PSTR param, int cmd)
{
	reg_myctl ();

	create_form ();

	HWND hwnd = i32create(TEXT("form"), "n|t|s|w|h|a|+x|+y|bc", "setbox",
			TEXT("对话 Dog"), WS_OVERLAPPEDWINDOW, 350, 350,
			"c", 100, 50, 0xffffff);
	i32setproc (hwnd, WM_SIZE, setbox_onsize);

	i32box ("dlg-top", hwnd);
	i32set (i32("dlg-top"), "bc", RGB(89,156,238));

	ShowWindow (hwnd, SW_SHOW);

	i32loop();
	return 0;
}
