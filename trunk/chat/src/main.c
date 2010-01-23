#include "i32.h"
#include "myctl.h"

int mainform_onclose (I32E e)
{
	PostQuitMessage(0);
	return 0;
}

int mainform_onsize (I32E e)
{
	i32vfill (e.hwnd,
		H(friendlist), -1,
		H(box1), -1,
		NULL
	);
	return 0;
}

int box1_onsize (I32E e)
{
	i32set(H(btnup), "a|-x", "c", 40);
	i32set(H(btndown), "a|+x", "c", 40);
	return 0;
}

int mainform_oncmd (I32E e)
{
	if (e.lp == H(btnup))
		SendMessage (H(friendlist), CLM_SCROLL, (WPARAM)47, 0);
	else
	if (e.lp == H(btndown))
		SendMessage (H(friendlist), CLM_SCROLL, (WPARAM)-47, 0);

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
	int r;

	hwnd = i32create ("form", "n|t|s|w|h|a|-x|+y", "mainform", "cat studio",
					  WS_OVERLAPPEDWINDOW, 300, 400, "c", 100, -10);
	i32setproc (hwnd, WM_DESTROY, mainform_onclose);
	i32setproc (hwnd, WM_SIZE, mainform_onsize);
	i32setproc (hwnd, WM_COMMAND, mainform_oncmd);

	i32create ("chatlist", "d|n|s|w|h|a|show", hwnd, "friendlist",
				WS_CTRL, 100, 100, "c", "y");
/**/
	i32box ("box1", hwnd);
	i32setproc (H(box1), WM_SIZE, box1_onsize);

	i32create("button", "d|n|t|s|w|h|a", H(box1), "btnup", "Up",
			WS_CTRL|BS_RADIOBUTTON, 70, 23, "c");

	i32create("button", "d|n|t|s|w|h|a", H(box1), "btndown", "Down",
			WS_CTRL|BS_CHECKBOX, 70, 23, "c");
	i32setproc (H(btndown), WM_CTLCOLORSTATIC, onclearbg);

	ShowWindow (hwnd, SW_SHOW);
}

int WINAPI WinMain (HINSTANCE hithis, HINSTANCE hiold, PSTR param, int cmd)
{
	reg_myctl ();

	create_form ();

	i32loop();


	return 0;
}
