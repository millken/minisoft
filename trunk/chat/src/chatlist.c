#include "myctl.h"

static LRESULT CALLBACK
winproc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)                  /* handle the messages */
    {
		case WM_PAINT: {
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);
			HBRUSH hbrush = CreateSolidBrush(0xff00ff);
			RECT r = {2, 2, 20, 20};
			FillRect (hdc, &r, hbrush);
			EndPaint(hwnd, &ps);
		}
		return 0;
        case WM_DESTROY:
            PostQuitMessage (0);       /* send a WM_QUIT to the message queue */
		return 0;
    }
	return DefWindowProc (hwnd, message, wParam, lParam);
}

void reg_chatlist ()
{
    WNDCLASSEX wincl;

    /* The Window structure */
    wincl.hInstance = GetModuleHandle(0);
    wincl.lpszClassName = "chatlist";
    wincl.lpfnWndProc = winproc;      /* This function is called by windows */
    wincl.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;      /* Catch double-clicks */
    wincl.cbSize = sizeof (WNDCLASSEX);

    /* Use default icon and mouse-pointer */
    wincl.hIcon = NULL;
    wincl.hIconSm = NULL;
    wincl.hCursor = LoadCursor (NULL, IDC_ARROW);
    wincl.lpszMenuName = NULL;                 /* No menu */
    wincl.cbClsExtra = 0;                      /* No extra bytes after the window class */
    wincl.cbWndExtra = 0;                      /* structure or the window instance */
    /* Use Windows's default colour as the background of the window */
    wincl.hbrBackground = (HBRUSH) COLOR_BTNHILIGHT;

    /* Register the window class, and if it fails quit the program */
    RegisterClassEx (&wincl);
}
