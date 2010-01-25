#include "i32.h"
#include "c.h"

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

	HWND hwnd = i32create (TEXT("form"), "n|t|s|w|h|a|tp", "bigform", TEXT("지 못한哈哈Эучены"),
				WS_OVERLAPPEDWINDOW
				, 300, 400, "c", "y");
	i32setproc (hwnd, WM_DESTROY, mainform_onclose);
	i32setproc (hwnd, WM_SIZE, bigform_onsize);

	i32box ("panel-t", hwnd);

	i32box ("panel-b", hwnd);

	i32create (TEXT("chatlist"), "d|n|s|w|h|a|show", hwnd, "friendlist",
				WS_CTRL|WS_VSCROLL, 100, 100, "c", "y");


	ShowWindow (hwnd, SW_SHOW);
}

int WINAPI WinMain (HINSTANCE hithis, HINSTANCE hiold, PSTR param, int cmd)
{
	reg_myctl ();

	create_form ();

	i32loop();

	return 0;
}
