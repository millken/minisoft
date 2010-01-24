#include "i32.h"
#include "c.h"

int mainform_onclose (I32E e)
{
	PostQuitMessage(0);
	return 0;
}

int mainform_onsize (I32E e)
{
	i32vfill (e.hwnd,
		i32("box-top"), -1,
		i32("box-foot"), -1,
		NULL
	);
	return 0;
}

int boxtop_onsize (I32E e)
{
	i32vfill (e.hwnd,
		i32("friendlist"), -1,
		NULL
	);
	return 0;
}

int boxfoot_onsize (I32E e)
{
	i32set(H(btnup), "a|-x", "c", 40);
	i32set(H(btndown), "a|+x", "c", 40);
	return 0;
}

int mainform_oncmd (I32E e)
{
	DWORD col = 0xffffff;
	HBRUSH hbrush;

	if ((HWND)e.lp == H(btnup))
		col = 0x33ffff;
	else
	if ((HWND)e.lp == H(btndown))
		col = 0xffff33;

	hbrush = CreateSolidBrush(col);
	SetClassLong(i32("box-foot"), GCL_HBRBACKGROUND, hbrush);
	InvalidateRect (i32("box-foot"), NULL, TRUE);

	return 0;
}

int onclearbg (I32E e)
{
	printf ("d\n");
	return 0;
}

void create_form ()
{
	HWND hwnd;

	hwnd = i32create ("form", "n|t|s|w|h|a|-x|+y|tp", "mainform", "Œ“",
					  WS_OVERLAPPEDWINDOW, 300, 400, "c", 100, -10, "yes");
	i32setproc (hwnd, WM_DESTROY, mainform_onclose);
	i32setproc (hwnd, WM_SIZE, mainform_onsize);
	i32setproc (hwnd, WM_COMMAND, mainform_oncmd);

	i32box("box-top", hwnd);
	i32set(i32("box-top"), "tp", "yes");
	i32setproc (i32("box-top"), WM_SIZE, boxtop_onsize);
	i32create ("chatlist", "d|n|s|w|h|a|show", i32("box-top"), "friendlist",
				WS_CTRL|WS_VSCROLL, 100, 100, "c", "y");

	i32box ("box-foot", hwnd);

	i32setproc (i32("box-foot"), WM_SIZE, boxfoot_onsize);

	i32create("button", "d|n|t|s|w|h|a", H(box-foot), "btnup", "Up",
			WS_CTRL, 70, 23, "c");

	i32create("button", "d|n|t|s|w|h|a", H(box-foot), "btndown", "Down",
			WS_CTRL, 70, 23, "c");

	ShowWindow (hwnd, SW_SHOW);
}

int WINAPI WinMain (HINSTANCE hithis, HINSTANCE hiold, PSTR param, int cmd)
{
	reg_myctl ();

	create_form ();

	i32loop();

	return 0;
}
