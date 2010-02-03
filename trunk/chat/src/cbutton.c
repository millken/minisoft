/*
 * cbutton 我的按钮控件
 */

#include "i32.h"
#include "ctrls.h"

#define DEF_MARGIN 4
#define TITLE_MARGIN 4
#define DEF_BCOLOR 0xffffff
#define DEF_BCOLOR_HOVER RGB(241,245,252)
#define DEF_BCOLOR_PUSHED RGB(239,242,248)
#define DEF_FCOLOR RGB(178,289,204)
#define DEF_RADIUS 4

static
enum {
	NONE = 0,
	HOVER,
	PUSHED
};

#define BTSIZE 64

static struct button {
	HWND hwnd;

	HBITMAP hbmp;
	HFONT hfont; /* 自定义控件貌似要自己做WM_SETFONT */
	int rad; /* 圆角半径 */

	int margin; /* 左右边距 */

	DWORD bcolor; /* 默认背景色 */
	DWORD bcolor_h; /* 经过时背景色 */
	DWORD bcolor_p; /* 按下后背景色 */
	DWORD fcolor; /* 边框色 */

	int status; /* 鼠标经过状态 0:none, 1:经过, 2:按下 */
	int oldstatus;

	struct button *next;

} *g_buttontable[BTSIZE]; /* 哈系表 */


static void init ()
{
	memset(g_buttontable, 0, sizeof(g_buttontable));
}

static void button_new (HWND hwnd)
{
	struct button **lp, *p;

	lp = &g_buttontable[(unsigned)hwnd%BTSIZE];
	while (*lp) {
		p = *lp;
		if (p->hwnd == hwnd)
			return;
		lp = &p->next;
	}

	*lp = (struct button *)i32malloc(sizeof(struct button));
	p = *lp;
	memset(p, 0, sizeof(struct button));
	p->hwnd = hwnd;
	p->bcolor = DEF_BCOLOR;
	p->bcolor_h = DEF_BCOLOR_HOVER;
	p->bcolor_p = DEF_BCOLOR_PUSHED;
	p->fcolor = DEF_FCOLOR;
	p->rad = DEF_RADIUS;
	p->margin = DEF_MARGIN;
}

static struct button *button_get (HWND hwnd)
{
	struct button *p;
	unsigned hashcode = (unsigned)hwnd % BTSIZE;

	for (p = g_buttontable[hashcode]; p; p = p->next)
		if (p->hwnd == hwnd)
			return p;
	return NULL;
}

static void button_del (HWND hwnd)
{
	struct button **lp, *p;
	unsigned hashcode = (unsigned)hwnd % BTSIZE;

	lp = &g_buttontable[hashcode];
	while (p = *lp) {
		struct button *next = p->next;
		if (p->hwnd == hwnd) {
			i32free(p);
			*lp = next;
			return;
		}
		lp = &next;
	}
}

static void draw_button (HWND hwnd, HDC hdc)
{
	RECT r;
	int buttonw = DEF_MARGIN*2, buttonh;
	TCHAR text[32];
	int titlelen = GetWindowText(hwnd, text, -1);
	int titlew, titleh, left = 0;
	BITMAP bmp;
	struct button *b;

	buttonh = i32clienth(hwnd);

	b = button_get(hwnd);
	if (!b) return;

	SelectObject(hdc, b->hfont);

	/* 计算按钮宽度 */ {
	SIZE tsize;
	left += DEF_MARGIN;
	GetTextExtentPoint32 (hdc, text, titlelen, &tsize);
	titlew = tsize.cx;
	titleh = tsize.cy;
	buttonw += titlew;

	if (b->hbmp) {
		GetObject(b->hbmp, sizeof(bmp), &bmp);
		buttonw += bmp.bmWidth;
		buttonw += TITLE_MARGIN;
	}
	buttonw += b->margin*2;
	i32set(hwnd, "w", buttonw);
	}

	/* 画背景 */
	if (b->status > 0) {
		HPEN hpen = CreatePen(PS_SOLID, 1, b->fcolor);
		HBRUSH hbrush = CreateSolidBrush(b->status==HOVER?b->bcolor_h:b->bcolor_p);
		int inner = 0;
		if (b->status == PUSHED)
			inner = 1;
		SelectObject(hdc, hpen);
		SelectObject(hdc, hbrush);
		RoundRect(hdc, inner, inner, buttonw-inner, buttonh-inner, b->rad, DEF_RADIUS);
		DeleteObject(hbrush);
		DeleteObject(hpen);
	}

	left += b->margin;

	/* 画图标 */
	if (b->hbmp) {
		i32blt(hdc, b->hbmp, left, (buttonh-bmp.bmHeight)/2);
		left += bmp.bmWidth;
		left += TITLE_MARGIN;
	}

	/* 画标题 */
	SetBkMode(hdc, TRANSPARENT);
	TextOut (hdc, left, (buttonh-titleh)/2, text, titlelen);
}

static LRESULT CALLBACK
win_proc (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	struct button *b = button_get(hwnd);

	switch (msg) {
		case WM_CREATE:
			button_new(hwnd);
		return 0;

		case WM_DESTROY:
			button_del(hwnd);
		return 0;

		case WM_ERASEBKGND:
		return 0;

		case WM_MOUSEMOVE: {
			if (b->status != PUSHED)
				b->status = HOVER;
			if (b->oldstatus != b->status) {
				InvalidateRect(hwnd, NULL, TRUE);
				b->oldstatus = b->status;
			}
			{
				TRACKMOUSEEVENT tme;
				tme.cbSize = sizeof(tme);
				tme.dwFlags = TME_LEAVE;
				tme.dwHoverTime = 1500;
				tme.hwndTrack = hwnd;

				TrackMouseEvent(&tme);
			}
		}
		return 0;

		case WM_MOUSELEAVE: {
			b->status = NONE;
			if (b->oldstatus != b->status) {
				InvalidateRect(hwnd, NULL, TRUE);
				b->oldstatus = b->status;
			}
		}
		return 0;

		case WM_LBUTTONDOWN:
			b->status = PUSHED;
			if (b->oldstatus != b->status) {
				InvalidateRect(hwnd, NULL, TRUE);
				b->oldstatus = b->status;
			}
		return 0;

		case WM_LBUTTONUP:
			if (b) b->status = HOVER;
			if (b->oldstatus != b->status) {
				InvalidateRect(hwnd, NULL, TRUE);
				b->oldstatus = b->status;
			}
			/* 通知父窗口 */ {
			int id = GetDlgCtrlID(hwnd);
			SendMessage (GetParent(hwnd), WM_COMMAND, (CBM_LCLICK<<16)|id, hwnd);
			}
		return 0;

		case WM_RBUTTONUP: {
			/* 通知父窗口 */ {
			int id = GetDlgCtrlID(hwnd);
			SendMessage (GetParent(hwnd), WM_COMMAND, (CBM_RCLICK<<16)|id, hwnd);
			}
		}
		return 0;

		case WM_PAINT: {
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);
			HDC hmem;
			HBITMAP hbmp;
			RECT r, dr;

			GetClientRect (hwnd, &r);

			hbmp = CreateCompatibleBitmap (hdc, r.right, r.bottom);
			hmem = CreateCompatibleDC (hdc);
			SelectObject (hmem, hbmp);

			/* 把遮住的父窗口再画一遍实现透明背景 */
			i32dadrect (hwnd, &dr);
			BitBlt (hmem, 0, 0, r.right, r.bottom, hdc, dr.left, dr.top, SRCCOPY);

			i32fillrect(hmem, &r, b->bcolor);
			draw_button(hwnd, hmem);
			BitBlt (hdc, 0, 0, r.right, r.bottom, hmem, 0, 0, SRCCOPY);

			DeleteObject(hbmp);
			DeleteObject(hmem);
			EndPaint(hwnd, &ps);
		}
		return 0;

		/* 输入接口 */

		case WM_SETICON: {
			if (b->hbmp)
				DeleteObject(b->hbmp);
			b->hbmp = (HBITMAP)wp;
		}
		return 0;

		case WM_SETFONT: {
			if (b->hfont)
				DeleteObject(b->hfont);
			b->hfont = (HFONT)wp;
		}
		return 0;

		case CBM_SETRADIUS:
			b->rad = (int)wp;
		return 0;

		case CBM_SETMARGIN:
			b->margin = (int)wp;
		return 0;

		case CBM_SETBCOLOR:
			b->bcolor = (DWORD)wp;
		return 0;

		case CBM_SETBCOLOR_HOVER:
			b->bcolor_h = (DWORD)wp;
		return 0;

		case CBM_SETBCOLOR_PUSHED:
			b->bcolor_p = (DWORD)wp;
		return 0;

		case CBM_SETFCOLOR:
			b->fcolor = (DWORD)wp;
		return 0;

	}

	return DefWindowProc(hwnd, msg, wp, lp);
}


void reg_cbutton ()
{
    WNDCLASSEX wincl;

    /* The Window structure */
    wincl.hInstance = GetModuleHandle(0);
    wincl.lpszClassName = TEXT("cbutton");
    wincl.lpfnWndProc = win_proc;      /* This function is called by windows */
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
