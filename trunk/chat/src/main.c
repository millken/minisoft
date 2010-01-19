#include "i32.h"
#include "myctl.h"


int f1_onsize (I32E e)
{
	i32callold(e);

	i32fillv (e.hwnd,
		H(flist1), -1,
		H(b1), 60,
		NULL);

	return 0;
}

int f1_onquit (I32E e)
{
	PostQuitMessage(0);
	return 0;
}

int f1_oncmd (I32E e)
{
	if (e.lp == H(bt1))
		SendMessage(H(flist1), GLM_SETITEM_GID, 77, 3);
	else
	if (e.lp == H(bt2))
		SendMessage(H(flist1), GLM_SETITEM_GID, 77, 1);
	return 0;
}


int b1_onsize (I32E e)
{
	i32set(H(bt1), "a|-x", "c", 40);
	i32set(H(bt2), "a|+x", "c", 40);

	return 0;
}


int WINAPI WinMain (HINSTANCE hithis, HINSTANCE hiold, PSTR param, int cmd)
{
	HWND hwnd;

	reg_myctl ();

	hwnd = i32create ("form", "n|t|s|w|h|a|-x|+y", "f1", "cat studio",
			WS_OVERLAPPEDWINDOW, 200, 300, "c", -100, 10);
	i32setproc (hwnd, WM_SIZE, f1_onsize);
	i32setproc (hwnd, WM_DESTROY, f1_onquit);
	i32setproc (hwnd, WM_COMMAND, f1_oncmd);



	i32create("grouplist", "n|d|s|w|h|a", "flist1", hwnd, WS_CHILD|WS_BORDER|WS_VISIBLE|WS_VSCROLL,
		100, 100, "c");
	ScrollWindow (hwnd, 0, 1, NULL, NULL);

	Group g = {1, "ºÇºÇ"};
	SendMessage (H(flist1), GLM_ADDGROUP, (WPARAM)&g, 0);
	g.name = "¹þ¹þ";
	//g.gid = 2;
	SendMessage (H(flist1), GLM_ADDGROUP, (WPARAM)&g, 0);
	g.gid = 3;
	SendMessage (H(flist1), GLM_ADDGROUP, (WPARAM)&g, 0);
	g.gid = 4;
	SendMessage (H(flist1), GLM_ADDGROUP, (WPARAM)&g, 0);
	g.gid = 5;
	SendMessage (H(flist1), GLM_ADDGROUP, (WPARAM)&g, 0);


	SendMessage (H(flist1), GLM_DELGROUP, (WPARAM)2, 0);
	//SendMessage (H(flist1), GLM_DELGROUP, (WPARAM)3, 0);

	SendMessage (H(flist1), GLM_RENAMEGROUP, (WPARAM)3, "dog");

	Item it;
	memset(&it, 0, sizeof(it));
	it.id = 33; it.gid = 1;
	it.title = "ÁùÁãÁù";
	SendMessage (H(flist1), GLM_ADDITEM, (WPARAM)&it, 0);
	it.id=77; it.title = "°×°ß²¡"; it.gid = 1;
	SendMessage (H(flist1), GLM_ADDITEM, (WPARAM)&it, 0);
	it.id = 88; it.title = "¸÷¸ö¸Ð"; it.gid = 3;
	SendMessage (H(flist1), GLM_ADDITEM, (WPARAM)&it, 0);
	it.id = 18; it.title = "¸÷¸ö¸Ð"; it.gid = 3;
	SendMessage (H(flist1), GLM_ADDITEM, (WPARAM)&it, 0);

	//SendMessage (H(flist1), GLM_DELITEM, (WPARAM)77, 0);
	//SendMessage (H(flist1), GLM_DELITEM, (WPARAM)99, 0);
	//SendMessage (H(flist1), GLM_DELITEM, (WPARAM)88, 0);
	SendMessage (H(flist1), GLM_GETITEM, (WPARAM)99, (LPARAM)&it);
	printf ("get: %d %s\n", it.id, it.title);

	SendMessage (H(flist1), GLM_GETITEM, (WPARAM)77, (LPARAM)&it);
	printf ("get: %d %s\n", it.id, it.title);

	SendMessage (H(flist1), GLM_SETITEM_GID, (WPARAM)89, (LPARAM)444);

	SendMessage (H(flist1), GLM_GETITEM, (WPARAM)88, (LPARAM)&it);
	printf ("get: %d %s\n", it.gid, it.title);


	i32box("b1", hwnd);
	i32setproc (H(b1), WM_SIZE, b1_onsize);
		i32create ("button", "n|d|s|w|h|a|t", "bt1", H(b1), WS_CHILD|WS_BORDER|WS_VISIBLE|BS_PUSHBUTTON,
				70, 23, "c", "Add");

		i32create ("button", "n|d|s|w|h|a|t", "bt2", H(b1), WS_CHILD|WS_BORDER|WS_VISIBLE,
				70, 23, "c", "Del");

	ShowWindow(hwnd, SW_SHOWNORMAL);
	i32loop();

	return 0;
}
