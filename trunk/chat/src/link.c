#include "i32.h"
#include "ctrls.h"

/* 点击颜色 */
#define COLOR_NORMAL 0xff0000
#define COLOR_HOVER 0x0000ff
#define COLOR_PUSH 0x0000ff

enum {
	NORMAL,
	HOVER,
	PUSH
};

/* 附加属性表 */
#define PROPSIZE 128

struct prop {
	HWND hwnd; /* key */
	HFONT hfont;
	DWORD color; /* normal color */
	DWORD color_h; /* hover color */
	DWORD color_p; /* push color */

	struct prop *next;
} *g_table[PROPSIZE];

static void init ()
{
	memset(g_table, 0, sizeof(g_table));
}

static struct prop *get_prop (HWND hwnd)
{
	struct prop **lp, *p;
	unsigned code = (unsigned)hwnd;

	lp = &g_table[code%PROPSIZE];
	while (*lp) {
		p = *lp;
		if (p->hwnd == hwnd)
			return p;
		lp = &p->next;
	}

	/* new */
	*lp = (struct prop*)malloc(sizeof(struct prop));
	p = *lp;
	memset (p, 0, sizeof(struct prop));
	p->hwnd = hwnd;
	p->color = COLOR_NORMAL;
	p->color_h = COLOR_HOVER;
	p->color_p = COLOR_PUSH;
	return p;
}


static void drawtext (HWND hwnd, HDC hdc, int state)
{
	struct prop *pp = get_prop(hwnd);
	DWORD color = 0;
	TCHAR title[128];

	if (!pp) return;

	switch (state) {
		case NORMAL: color = pp->color; break;
		case HOVER: color = pp->color_h; break;
		case PUSH: color = pp->color_p; break;
	}

	SetBkMode(hdc, TRANSPARENT);
	SelectObject (hdc, pp->hfont);
	GetWindowText(hwnd, title, sizeof(title)/sizeof(TCHAR));
	i32textout (hdc, 0, 0, title, color);
}

static CALLBACK LRESULT winproc (HWND hwnd, UINT message, WPARAM wp, LPARAM lp)
{
	static BOOL entered = FALSE;
	struct prop *pp;

	switch (message) {
		case WM_CREATE:
			get_prop(hwnd);
		return 0;

		case WM_ERASEBKGND:
		return 0;

		case WM_PAINT: {
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);
			drawtext (hwnd, hdc, NORMAL);
			EndPaint(hwnd, &ps);
		}
		return 0;

		case WM_MOUSEMOVE: {
			HDC hdc;
			if (!entered) {
				hdc = GetWindowDC(hwnd);
				drawtext (hwnd, hdc, HOVER);
				ReleaseDC(hwnd, hdc);
				entered = TRUE;
			}
		}
		{
			TRACKMOUSEEVENT tme;
			tme.cbSize = sizeof(tme);
			tme.dwFlags = TME_LEAVE;
			tme.dwHoverTime = 1500;
			tme.hwndTrack = hwnd;

			TrackMouseEvent(&tme);
		}
		return 0;

		case WM_MOUSELEAVE: {
			HDC hdc = GetWindowDC(hwnd);
			drawtext (hwnd, hdc, NORMAL);
			ReleaseDC(hwnd, hdc);
			entered = FALSE;
		}
		return 0;

		case WM_LBUTTONDOWN: {
			HDC hdc = GetWindowDC(hwnd);
			drawtext (hwnd, hdc, PUSH);
			ReleaseDC(hwnd, hdc);
			i32set(hwnd, "+y", 1);
		}
		return 0;

		case WM_LBUTTONUP: {
			HDC hdc = GetWindowDC(hwnd);
			drawtext (hwnd, hdc, HOVER);
			ReleaseDC(hwnd, hdc);
			i32set(hwnd, "-y", 1);
		}
		return 0;

		case WM_SETFONT: {
			pp = get_prop(hwnd);
			if (pp) {
				if (pp->hfont)
					DeleteObject(pp->hfont);
				pp->hfont = (HFONT)wp;
			}
		}
		return 0;

		/* 接口 */
		case LM_SETCOLOR: {
			pp = get_prop(hwnd);
			if (pp)
			pp->color = (DWORD)wp;
		}
		return 0;

		case LM_SETCOLOR_HOVER: {
			pp = get_prop(hwnd);
			if (pp)
			pp->color_h = (DWORD)wp;
		}
		return 0;

		case LM_SETCOLOR_PUSH: {
			pp = get_prop(hwnd);
			if (pp)
			pp->color_p = (DWORD)wp;
		}
		return 0;
	}
	return DefWindowProc(hwnd, message, wp, lp);
}

void reg_hyperlink ()
{
    WNDCLASSEX wincl;

    /* The Window structure */
    wincl.hInstance = GetModuleHandle(0);
    wincl.lpszClassName = TEXT("linker");
    wincl.lpfnWndProc = winproc;      /* This function is called by windows */
    wincl.style = CS_HREDRAW | CS_VREDRAW;      /* Catch double-clicks */
    wincl.cbSize = sizeof (WNDCLASSEX);

    /* Use default icon and mouse-pointer */
    wincl.hIcon = NULL;
    wincl.hIconSm = NULL;
    wincl.hCursor = LoadCursor (NULL, IDC_HAND);
    wincl.lpszMenuName = NULL;                 /* No menu */
    wincl.cbClsExtra = 0;                      /* No extra bytes after the window class */
    wincl.cbWndExtra = 0;                      /* structure or the window instance */
    /* Use Windows's default colour as the background of the window */
    wincl.hbrBackground = (HBRUSH) COLOR_BACKGROUND;

    /* Register the window class, and if it fails quit the program */
    RegisterClassEx (&wincl);

    init();
}

HWND create_link (HWND dad, char *format, ...)
{
	HWND hwnd = i32create(TEXT("linker"), "d|s|w|h|f",
		dad, WS_CTRL, 80, 17, "Arial,14,0,0,1");
	if (!hwnd) return NULL;

	if (format) {
		va_list p;
		va_start (p, format);
		i32vset (hwnd, format, p);
		va_end(p);
	}

	return hwnd;
}

void set_linkcolor (HWND hwnd, DWORD color, DWORD colorh, DWORD colorp)
{
	i32send (hwnd, LM_SETCOLOR, color, 0);
	i32send (hwnd, LM_SETCOLOR_HOVER, colorh, 0);
	i32send (hwnd, LM_SETCOLOR_PUSH, colorp, 0);
}
