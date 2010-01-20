#include "i32.h"
#include "myctl.h"

int mainform_onclose (I32E e)
{
	PostQuitMessage(0);
	return 0;
}

void create_form ()
{
	HWND hwnd;

	hwnd = i32create ("form", "n|t|s|w|h|a|-x|+y", "mainform", "cat studio",
					  WS_OVERLAPPEDWINDOW, 200, 300, "c", 100, 200);
	i32setproc (hwnd, WM_DESTROY, mainform_onclose);

	i32create ("chatlist", "d|n|s|w|h|a", hwnd, "friendlist",
				WS_CTRL, 100, 100, "c");

	//ShowWindow (hwnd, SW_SHOW);
}

int WINAPI WinMain (HINSTANCE hithis, HINSTANCE hiold, PSTR param, int cmd)
{
	reg_myctl ();

	create_form ();

	//i32loop();
	return 0;
}
