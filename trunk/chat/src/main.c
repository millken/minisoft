#include <windows.h>
#include "i32.h"
#include "myctl.h"

LRESULT CALLBACK WindowProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)                  /* handle the messages */
    {
        case WM_DESTROY:
            PostQuitMessage (0);       /* send a WM_QUIT to the message queue */
		return 0;
    }
	return DefWindowProc (hwnd, message, wParam, lParam);
}

void reg()
{
    WNDCLASSEX wincl;        /* Data structure for the windowclass */

    /* The Window structure */
    wincl.hInstance = GetModuleHandle(0);
    wincl.lpszClassName = "form";
    wincl.lpfnWndProc = WindowProcedure;      /* This function is called by windows */
    wincl.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;                 /* Catch double-clicks */
    wincl.cbSize = sizeof (WNDCLASSEX);

    /* Use default icon and mouse-pointer */
    wincl.hIcon = LoadIcon (NULL, IDI_APPLICATION);
    wincl.hIconSm = LoadIcon (NULL, IDI_APPLICATION);
    wincl.hCursor = LoadCursor (NULL, IDC_ARROW);
    wincl.lpszMenuName = NULL;                 /* No menu */
    wincl.cbClsExtra = 0;                      /* No extra bytes after the window class */
    wincl.cbWndExtra = 0;                      /* structure or the window instance */
    /* Use Windows's default colour as the background of the window */
    wincl.hbrBackground = (HBRUSH) COLOR_BACKGROUND;

    /* Register the window class, and if it fails quit the program */
    RegisterClassEx (&wincl);
}


int click (HWND hwnd, WPARAM wp, LPARAM lp)
{
	printf("click\n");
	MessageBox (hwnd, "no", "just", MB_OK);
	return 0;
}

int hover (HWND hwnd, WPARAM wp, LPARAM lp)
{
	printf("hover\n");
}

int button_draw (HWND hwnd, WPARAM wp, LPARAM lp)
{
	PAINTSTRUCT ps;
	HDC hdc;
	HBRUSH hbrush = CreateSolidBrush(0xff00ff);
	hdc = BeginPaint(hwnd, &ps);
	SetBkColor(hdc, TRANSPARENT);
	//FillRect (hdc, &i32rect(1, 1, 5, 5), hbrush);
	EndPaint(hwnd, &ps);
	return 0;
}

int setdns (HWND hwnd, WPARAM wp, LPARAM lp)
{
	if (hwnd==i32h("b3"))
		printf ("b3\n");
	else if (hwnd==i32h("b4"))
		printf ("b4\n");
}

int WINAPI WinMain (HINSTANCE hiold, HINSTANCE hithis, LPSTR param, int cmd)
{
	HWND hwnd, hbutton;
	I32PROC f;

	reg();
	reg_myctl ();

	i32create("form", "name|title", "f13b", "i33");
	i32set (i32h("f13b"), "style|name|x|y|w|h|size|align",
		WS_OVERLAPPEDWINDOW, "f1", 10, 10, 200, 290,
		i32pair(220, 300), "center");
	hwnd = i32h("f1");
	i32set (hwnd, "-style", WS_MAXIMIZEBOX);

	i32create("button", "dad|name|title|style|rect",
		i32h("f13b"),
		"b1", "bb", WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX|BS_FLAT,
		i32rect(10, 20, 70, 20));
	i32set (i32h("b1"), "x", 10);
	i32set (i32h("b1"), "align", "top");

	i32create("button", "dad|name|title|style|rect",
		i32h("f13b"),
		"b2", "bb", WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,
		i32rect(10, 50, 70, 20));
	i32set (i32h("b2"), "rect", i32rect(20, 50, 70, 20));
	i32create("button", "dad|name|title|style|rect",
		i32h("f13b"),
		"b3", "bb", WS_CHILD|WS_VISIBLE|BS_AUTORADIOBUTTON,
		i32rect(10, 80, 70, 20));
	i32create("button", "dad|name|title|style|rect",
		i32h("f13b"),
		"b4", "bb", WS_CHILD|WS_VISIBLE|BS_AUTORADIOBUTTON,
		i32rect(10, 110, 70, 20));
	i32create("button", "dad|name|title|style|rect",
		i32h("f13b"),
		"b5", "bb", WS_CHILD|WS_VISIBLE|BS_AUTORADIOBUTTON,
		i32rect(10, 140, 70, 20));

	i32setproc (i32h("b3"), WM_LBUTTONDOWN, setdns);
	i32setproc (i32h("b4"), WM_LBUTTONDOWN, setdns);

	hbutton = i32h("b1");
	HDC hdc = GetDC(hbutton);
	SetBkColor (hdc, GetSysColor (COLOR_BTNSHADOW)) ;
	i32setproc (hbutton, WM_ERASEBKGND, button_draw);

	i32setproc(hwnd, WM_LBUTTONDOWN, click);
	i32setproc(hwnd, WM_LBUTTONUP, hover);

	//////////
	i32create("chatlist", "dad|name|style|x|y|w|h", i32h("f13b"), "ct1",
			WS_CHILD|WS_VISIBLE|WS_BORDER,
			10, 10, 120, 100);


#if 1
	{
		DWORD t1 = GetTickCount();
		int i;
		for (i = 0; i < 100000; i++) {
			//i32set (i32h("f13b"), "align|name|x|y|w|h", "left", "f1", 10, 10, 40, 290);
			//i32set(hwnd, "w", 100);
			//i32set(hwnd, "size", i32pair(100, 100));
			//i32set(hwnd, "pos", i32pair(100, 100));
			//i32set(hwnd, "title", "hehe");
			//i32set (hwnd, "align", "top|left");
			//i32set (hwnd, "+style", WS_MINIMIZEBOX);
			//i32set (hwnd, "align|+style", "top|right|center",WS_MINIMIZEBOX);
			//SetWindowLong(hwnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);
			//float x = 1.4*i*(3.14);
			//i32set (hwnd, "style", WS_VISIBLE);
			//i32getproc (hwnd, WM_LBUTTONDOWN);
			//i32h("f13b");
		}
		printf ("time: %u\n", GetTickCount()-t1);
	}
#endif

	ShowWindow (hwnd, SW_SHOW);

	//i32debug ();


	i32msgloop();

	return 0;
}
