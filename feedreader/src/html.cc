/*
 * 附加的html浏览控件
 */

#include "i32.h"
#include <commdlg.h>
#include "htmlayout/htmlayout.h"
#include "htmlayout/behaviors/notifications.h" // hyperlink behavior notfication is here
#include "html.h"

static CALLBACK LRESULT win_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	BOOL bhandled;
	LRESULT result = HTMLayoutProcND(hwnd,msg,wp,lp, &bhandled);
	if (bhandled)
		return result;

	return DefWindowProc(hwnd, msg, wp, lp);
}

void reg_html_control ()
{
    WNDCLASSEX wincl;

    /* The Window structure */
    wincl.hInstance = GetModuleHandle(0);
    wincl.lpszClassName = TEXT("html");
    wincl.lpfnWndProc = win_proc;      /* This function is called by windows */
    wincl.style = CS_HREDRAW | CS_VREDRAW;      /* Catch double-clicks */
    wincl.cbSize = sizeof (WNDCLASSEX);

    /* Use default icon and mouse-pointer */
    wincl.hIcon = NULL;
    wincl.hIconSm = NULL;
    wincl.hCursor = LoadCursor (NULL, IDC_ARROW);
    wincl.lpszMenuName = NULL;                 /* No menu */
    wincl.cbClsExtra = 0;                      /* No extra bytes after the window class */
    wincl.cbWndExtra = 0;                      /* structure or the window instance */
    /* Use Windows's default colour as the background of the window */
    wincl.hbrBackground = (HBRUSH) COLOR_BACKGROUND;

    /* Register the window class, and if it fails quit the program */
    RegisterClassEx (&wincl);
}

HWND html_create (HWND dad, char *format, ...)
{
	HWND hwnd;
	va_list p;

	hwnd = i32create(TEXT("html"), "d|s", dad, WS_CTRL);
	if (!hwnd) return NULL;

	if (format) {
		va_start(p, format);
		i32vset (hwnd, format, p);
		va_end(p);
	}

	return hwnd;
}

int html_loadfile (HWND hwnd, wchar_t *filename)
{
	return HTMLayoutLoadFile(hwnd, filename);
	//char *html = "<a href='http://www.google.com/reader/view/'>haha</a>";
	//int e = HTMLayoutLoadHtml(hWnd, (LPCBYTE)html, strlen(html));
}

int html_loadstring (HWND hwnd, char *html)
{
	return HTMLayoutLoadHtml(hwnd, (LPCBYTE)html, strlen(html));
}
