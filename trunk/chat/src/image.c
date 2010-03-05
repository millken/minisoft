/*
 * image 图片控件
 */

#include "i32.h"
#include "ctrls.h"

#define DEF_PADDING 0
#define DEF_BCOLOR 0xffffff
#define DEF_BCOLOR_HOVER 0xfafafa
#define DEF_FCOLOR 0x777777
#define DEF_FCOLOR_HOVER 0x777777

enum {
	NONE = 0,
	HOVER,
	PUSHED
};

#define IMGTABLESIZE 128

static
struct inode {
	HWND hwnd;

	HBITMAP data;
	int padding; /* 内间距 */
	DWORD bcolor;
	DWORD bcolor_hover;
	DWORD fcolor;
	DWORD fcolor_hover;

	int status; /* 按下状态, 0:无, 1:经过, 2:按下 */
	int oldstatus;

	struct inode *next;

} *g_itable[IMGTABLESIZE]; /* one hwnd, one node */


static void init ()
{
	memset(g_itable, 0, sizeof(g_itable));
}

static void image_new (HWND hwnd)
{
	struct inode **lp, *p;

	lp = &g_itable[(unsigned)hwnd % IMGTABLESIZE];
	while (*lp) {
		p = *lp;
		if (p->hwnd == hwnd)
			return;
		lp = &p->next;
	}

	*lp = (struct inode *)i32malloc(sizeof(struct inode));
	p = *lp;
	memset (p, 0, sizeof(struct inode));
	p->hwnd = hwnd;
	p->padding = DEF_PADDING;
	p->bcolor = DEF_BCOLOR;
	p->bcolor_hover = DEF_BCOLOR_HOVER;
	p->fcolor = DEF_FCOLOR;
	p->fcolor_hover = DEF_FCOLOR_HOVER;
}

static struct inode *image_get (HWND hwnd)
{
	struct inode *p;

	for (p = g_itable[(unsigned)hwnd%IMGTABLESIZE]; p; p = p->next)
		if (p->hwnd == hwnd)
			return p;
	return NULL;
}

static void draw_image (HWND hwnd, HDC hdc, struct inode *img)
{
	RECT r;

	if (!img) return;

	GetClientRect (hwnd, &r);

	/* 画背景 */ {
		DWORD bcolor = img->status>0?img->bcolor_hover:img->bcolor;
		i32fillrect (hdc, &r, bcolor);
	}

	/* 画线框 */ {
		DWORD fcolor = img->status>0?img->fcolor_hover:img->fcolor;
		i32framerect(hdc, &r, fcolor);
	}

	/* 画图象 */
	if (img->data) {
		int b = img->padding;
		i32draw (hdc, img->data, b, b, r.right-b*2, r.bottom-b*2);
	}
}

static LRESULT CALLBACK
win_proc (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	struct inode *img = image_get(hwnd);

	switch (msg) {
		case WM_CREATE:
			image_new(hwnd);
		return 0;

		case WM_ERASEBKGND:
		return 0;

		case WM_PAINT: {
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);
			HDC hmem;
			HBITMAP hbmp;
			RECT r;

			GetClientRect(hwnd, &r);
			hbmp = CreateCompatibleBitmap (hdc, r.right, r.bottom);
			hmem = CreateCompatibleDC (hdc);
			SelectObject (hmem, hbmp);

			draw_image (hwnd, hmem, img);

			BitBlt (hdc, 0, 0, r.right, r.bottom, hmem, 0, 0, SRCCOPY);
			DeleteObject(hbmp);
			DeleteObject(hmem);

			EndPaint(hwnd, &ps);
		}
		return 0;

		case WM_LBUTTONDOWN:
			img->status = PUSHED;
			img->oldstatus = img->status;
			InvalidateRect(hwnd, NULL, TRUE);
		return 0;

		case WM_LBUTTONUP:
			img->status = HOVER;
			img->oldstatus = img->status;
			InvalidateRect(hwnd, NULL, TRUE);
		return 0;

		case WM_MOUSEMOVE:
		{
			if (img->status != PUSHED)
				img->status = HOVER;
			if (img->status != img->oldstatus) {
				img->oldstatus = img->status;
				InvalidateRect(hwnd, NULL, TRUE);
			}
		}{
			TRACKMOUSEEVENT tme;
			tme.cbSize = sizeof(tme);
			tme.dwFlags = TME_LEAVE;
			tme.hwndTrack = hwnd;

			TrackMouseEvent(&tme);
		}
		return 0;

		case WM_MOUSELEAVE: {
			img->status = NONE;
			if (img->status != img->oldstatus) {
				img->oldstatus = img->status;
				InvalidateRect(hwnd, NULL, TRUE);
			}
		}
		return 0;

		/* interface */

		case IM_SETIMAGE:
			if (img->data)
				DeleteObject(img->data);
			img->data = (HBITMAP)wp;
			InvalidateRect(hwnd, NULL, TRUE);
		return 0;

		case IM_SETPADDING:
			img->padding = (int)wp;
		return 0;

		case IM_SETBCOLOR:
			img->bcolor = (DWORD)wp;
		return 0;

		case IM_SETBCOLOR_HOVER:
			img->bcolor_hover = (DWORD)wp;
		return 0;

		case IM_SETFCOLOR:
			img->fcolor = (DWORD)wp;
		return 0;

		case IM_SETFCOLOR_HOVER:
			img->fcolor_hover = (DWORD)wp;
		return 0;
	}

	return DefWindowProc (hwnd, msg, wp, lp);
}

void reg_image ()
{
    WNDCLASSEX wincl;

    /* The Window structure */
    wincl.hInstance = GetModuleHandle(0);
    wincl.lpszClassName = TEXT("image");
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

	init();
}
