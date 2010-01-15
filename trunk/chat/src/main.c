#include "i32.h"
#include "myctl.h"

int form_onsize (I32EVENT e)
{
	i32fillh (e.hwnd,
		i(b1), 0,
		i(b1.5), 12,
		i(b2), -1,
		NULL);
	return 0;
}

int b1_onsize (I32EVENT e)
{
	i32fillv (e.hwnd,
		i(bt1), -1,
		i(b11), -1,
		i(b12), -1,
		i(b13), -1,
		NULL);
	return 0;
}

int b2_onsize (I32EVENT e)
{
	i32fillv (e.hwnd,
		i(b3), -1,
		i(b4), 19,
		i(b5), -1,
		NULL);
	return 0;
}

int b3_onsize (I32EVENT e)
{
	i32fillh (e.hwnd,
		i(b31), -1,
		i(b32), -1,
		i(b33), -1,
		i(b34), -1,
		NULL);
	return 0;
}

int b4_onsize (I32EVENT e)
{
	i32fillh (e.hwnd,
		i(b41), -1,
		i(b42), -1,
		i(b43), -1,
		NULL);
	return 0;
}

int on_clearbg (I32E e)
{
	printf ("f");
	i32callold(e);
	return 0;
}

int WINAPI WinMain (HINSTANCE hithis, HINSTANCE hiold, PSTR param, int cmd)
{
	HWND hwnd;

	hwnd = i32create ("box", "n|t|s|w|h|a|-x|+y", "form", "cat home",
			WS_OVERLAPPEDWINDOW, 200, 300, "c", -100, 10);
	i32setproc (hwnd, WM_SIZE, form_onsize);

	i32box("b1", hwnd);
	i32set(i(b1), "w|h|a|+y", 50, 50, "c", 40);
			i32create("button", "n|d|t|s|w|h|a", "bt1", i(b1), "ÄãºÃ..",
				WS_CHILD|WS_VISIBLE|BS_RADIOBUTTON, 76, 24, "c");
			i32setproc (i(bt1), WM_PAINT, on_clearbg);
			i32box("b11", i(b1));
			i32box("b12", i(b1));
			i32box("b13", i(b1));
			i32setproc (i(b1), WM_SIZE, b1_onsize);

	i32box("b1.5", hwnd);
	i32box("b2", hwnd);
		i32box("b3", i(b2));
			i32box("b31", i(b3));
			i32box("b32", i(b3));
			i32box("b33", i(b3));
			i32box("b34", i(b3));
			i32setproc (i(b3), WM_SIZE, b3_onsize);
		i32box("b4", i(b2));
			i32box("b41", i(b4));
			i32box("b42", i(b4));
			i32box("b43", i(b4));
			i32setproc (i(b4), WM_SIZE, b4_onsize);
		i32box("b5", i(b2));
		i32setproc (i(b2), WM_SIZE, b2_onsize);
/**/
	//SendMessage (hwnd, WM_SIZE, 0, 0);
	ShowWindow(hwnd, SW_SHOWNORMAL);
	UpdateWindow(hwnd);
	i32loop();

	return 0;
}
