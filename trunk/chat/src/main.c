#include <windows.h>
#include "i32.h"

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
    if (!RegisterClassEx (&wincl))
        return 0;
}

int msgloop()
{
	MSG messages;

    /* Run the message loop. It will run until GetMessage() returns 0 */
    while (GetMessage (&messages, NULL, 0, 0))
    {
        /* Translate virtual-key messages into character messages */
        TranslateMessage(&messages);
        /* Send message to WindowProcedure */
        DispatchMessage(&messages);
    }
}

int click (HWND hwnd, WPARAM wp, LPARAM lp)
{
	printf("click\n");
}

int hover (HWND hwnd, WPARAM wp, LPARAM lp)
{
	printf("hover\n");
}

int WINAPI WinMain (HINSTANCE hiold, HINSTANCE hithis, LPSTR param, int cmd)
{
	HWND hwnd;
	I32CALLBACK f;

	reg();

	i32create("form", "f1");
	//i32set (hwnd, ":s:n", WS_OVERLAPPED, "f2");
	hwnd = i32h("f1");

	i32set_callback(hwnd, WM_LBUTTONDOWN, click);
	i32set_callback(hwnd, WM_LBUTTONUP, hover);
	i32set_callback(hwnd, WM_LBUTTONDBLCLK, hover);
	i32set_callback(hwnd, WM_CANCELJOURNAL, hover);
	i32set_callback(hwnd, WM_RBUTTONDBLCLK, hover);
	f = i32get_callback(hwnd, WM_CANCELJOURNAL);
	f (hwnd, 0, 0);
	//ShowWindow (hwnd, SW_SHOW);
	//msgloop();

	return 0;
}
