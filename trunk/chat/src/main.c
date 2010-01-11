#include "i32.h"
#include "myctl.h"

void i32vfill (HWND hwnd, struct boxlist *l)
{
	//struct boxlist blist = {}
	printf("%d\n", l[0].v);
}

int WINAPI WinMain (HINSTANCE hithis, HINSTANCE hiold, PSTR param, int cmd)
{
	HWND hwnd;

	hwnd = i32create ("box", "n|t|s|w|h|a", "form", "cat home",
			WS_OVERLAPPEDWINDOW, 300, 200, "c");

	//i32create("box", "n", "b1");
	HWND hbox = i32box();
	//i32set();
	i32set (hbox, "d|s|w|h|a", hwnd, WS_CHILD|WS_BORDER|WS_VISIBLE,
		60, 40, "c");

	//i32box ();
	//HWND hbox = i32create_box (100, 100);
	//i32set(hbox, "n|a", "box", "c");
	struct boxlist blist[] = {
		{1, 98},
		{2, -1},
		{3, 10},
		{0, 0}
	};
	//i32vfill (hwnd, blist);
	//i32debug ();


	i32loop();

	return 0;
}
