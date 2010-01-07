#include "i32.h"
#include "myctl.h"


int WINAPI WinMain (HINSTANCE hiold, HINSTANCE hithis, LPSTR param, int cmd)
{
	HWND hwnd, hbutton;

	hwnd = i32create ("box", "title|style|w|h|align|show|layout", "cat home",
			WS_OVERLAPPEDWINDOW, 300, 200, "center", "yes",
			"margin:5 auto;padding:5;align:center;");

	ShowWindow (hwnd, SW_SHOW);

	//i32debug ();


	i32msgloop();

	return 0;
}
