#include "i32.h"
#include "myctl.h"


int f1_onsize (I32E e)
{
	i32callold(e);

	i32fillv (e.hwnd,
		H(flist1), -1,
		H(b1), -1,
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
	printf ("c");
	return 0;
}


int b1_onsize (I32E e)
{
	i32set(H(bt1), "a", "c");

	return 0;
}


int WINAPI WinMain (HINSTANCE hithis, HINSTANCE hiold, PSTR param, int cmd)
{
	HWND hwnd;

	reg_myctl ();

	hwnd = i32create ("form", "n|t|s|w|h|a|-x|+y", "f1", "cat home",
			WS_OVERLAPPEDWINDOW, 200, 300, "c", -100, 10);
	i32setproc (hwnd, WM_SIZE, f1_onsize);
	i32setproc (hwnd, WM_DESTROY, f1_onquit);
	i32setproc (hwnd, WM_COMMAND, f1_oncmd);

	i32create("grouplist", "n|d|s|w|h|a", "flist1", hwnd, WS_CHILD|WS_BORDER|WS_VISIBLE,
		100, 100, "c");

	Group g = {1, "บวบว"};
	SendMessage (H(flist1), FM_ADDGROUP, (WPARAM)&g, 0);

	i32box("b1", hwnd);
	i32setproc (H(b1), WM_SIZE, b1_onsize);
	  i32create ("button", "n|d|s|w|h|a|t", "bt1", H(b1), WS_CHILD|WS_BORDER|WS_VISIBLE,
			70, 24, "c", "Button");


/*
	i32create("flist", "xx", ...);
	groups = getservergroup(uid);
	friends = getserverfriend(uid);
	foreach (group in groups)
		SendMessage (flist, FM_ADDGROUP, {group.id, group.name}, 0);
	foreach (f in friends)
		SendMessage (flist, FM_ADDFRIEND, {f.uid, f.hicon, f.gid, f.number, f.name, f.bakname, f.sign}, 0);
	SendMessage (flist, FM_SETGROUPID, f.uid, f.gid);
	SendMessage (flist, FM_SETGROUPNAME, gid, "NAME");
	SendMessage (flist, FM_DELGROUP, gid, 0);
	SendMessage (flist, FM_FOLDGROUP, gid, 0);
	SendMessage (flist, FM_EXGROUP, gid, 0);
*/
	ShowWindow(hwnd, SW_SHOWNORMAL);
	i32loop();

	return 0;
}
