/*
 * batton 按钮控件
 */

#include "i32.h"
#include "ctrls.h"

static
enum {
	NONE = 0,
	HOVER,
	PUSHED
};

static enum {
	LEFT = 0,
	RIGHT
};

#define DEF_MARGIN 1
#define DEF_TITLEMARGIN 4
#define DEF_BCOLOR 0xff00ff
#define DEF_BCOLOR_HOVER 0xff00ff
#define DEF_BCOLOR_PUSHED 0xff00ff
#define DEF_FCOLOR 0xff00ff
#define DEF_RADIUS 0
#define DEF_ICONALIGN RIGHT

#define BTSIZE 64

static struct batton {
	HWND hwnd;

	HBITMAP hbmp;
	HFONT hfont; /* 自定义控件貌似要自己做WM_SETFONT */
	int rad; /* 圆角半径 */

	int margin; /* 左右边距 */
	int titlemargin; /* 图标与文字的距离 */
	int iconalign; /* 图标居左0还是居右1 */

	DWORD bcolor; /* 默认背景色 */
	DWORD bcolor_h; /* 经过时背景色 */
	DWORD bcolor_p; /* 按下后背景色 */
	DWORD fcolor; /* 边框色 */
	DWORD textcolor; /* 字体颜色 */

	int status; /* 鼠标经过状态 0:none, 1:经过, 2:按下 */
	int oldstatus;

	struct batton *next;

} *g_battontable[BTSIZE]; /* 哈系表 */


static void init ()
{
	memset(g_battontable, 0, sizeof(g_battontable));
}

static void batton_new (HWND hwnd)
{
	struct batton **lp, *p;

	lp = &g_battontable[(unsigned)hwnd%BTSIZE];
	while (*lp) {
		p = *lp;
		if (p->hwnd == hwnd)
			return;
		lp = &p->next;
	}

	*lp = (struct batton *)i32malloc(sizeof(struct batton));
	p = *lp;
	memset(p, 0, sizeof(struct batton));
	p->hwnd = hwnd;
	p->bcolor = DEF_BCOLOR;
	p->bcolor_h = DEF_BCOLOR_HOVER;
	p->bcolor_p = DEF_BCOLOR_PUSHED;
	p->fcolor = DEF_FCOLOR;
	p->rad = DEF_RADIUS;
	p->margin = DEF_MARGIN;
	p->titlemargin = DEF_TITLEMARGIN;
	p->iconalign = DEF_ICONALIGN;
}

static struct batton *batton_get (HWND hwnd)
{
	struct batton *p;
	unsigned hashcode = (unsigned)hwnd % BTSIZE;

	for (p = g_battontable[hashcode]; p; p = p->next)
		if (p->hwnd == hwnd)
			return p;
	return NULL;
}

static void batton_del (HWND hwnd)
{
	struct batton **lp, *p;
	unsigned hashcode = (unsigned)hwnd % BTSIZE;

	lp = &g_battontable[hashcode];
	while (p = *lp) {
		struct batton *next = p->next;
		if (p->hwnd == hwnd) {
			i32free(p);
			*lp = next;
			return;
		}
		lp = &next;
	}
}

static void draw_batton (HWND hwnd, HDC hdc)
{
	RECT r;
	int battonw = 0, battonh;
	TCHAR title[32];
	int titlelen = GetWindowText(hwnd, title, -1);
	int titlew, titleh, iconw, left = 0;
	BITMAP bmp;
	struct batton *b;
	int titlex, iconx;

	battonh = i32clienth(hwnd);

	b = batton_get(hwnd);
	if (!b) return;

	SelectObject(hdc, b->hfont);

	/* 计算按钮宽度 */ {
	SIZE tsize;
	GetTextExtentPoint32 (hdc, title, titlelen, &tsize);
	titlew = tsize.cx;
	titleh = tsize.cy;
	battonw += titlew;
	if (b->hbmp) {
		GetObject(b->hbmp, sizeof(bmp), &bmp);
		iconw = bmp.bmWidth;
		battonw += iconw;
		if (titlew>0)
			battonw += b->titlemargin;
	}
	battonw += b->margin*2;
	i32set(hwnd, "w", battonw);
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
		if (b->status==HOVER&&b->bcolor_h!=0xff00ff ||
			b->status==PUSHED&&b->bcolor_p!=0xff00ff)
		RoundRect(hdc, inner, inner, battonw-inner, battonh-inner, b->rad, b->rad);
		DeleteObject(hbrush);
		DeleteObject(hpen);
	}

	left += b->margin;

	/* 考虑对齐 */
	if (b->hbmp) {
		if (b->iconalign == RIGHT) {
			titlex = left;
			iconx = titlex + titlew + b->titlemargin;
		}
		else {
			iconx = left;
			titlex = iconx + iconw + b->titlemargin;
		}
	}
	else
		titlex = left;

	/* 画图标 */
	if (b->hbmp) {
		i32blt(hdc, b->hbmp, iconx, (battonh-bmp.bmHeight)/2);
	}

	/* 画标题 */
	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, b->textcolor);
	TextOut (hdc, titlex, (battonh-titleh)/2, title, titlelen);
}

static LRESULT CALLBACK
win_proc (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	struct batton *b = batton_get(hwnd);

	switch (msg) {
		case WM_CREATE:
			batton_new(hwnd);
		return 0;

		case WM_DESTROY:
			batton_del(hwnd);
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
			/* 通知父窗口 */ {
			int id = GetDlgCtrlID(hwnd);
			SendMessage (GetParent(hwnd), WM_COMMAND, (BM_LBUTTONDOWN<<16)|id, hwnd);
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
			SendMessage (GetParent(hwnd), WM_COMMAND, (BM_LBUTTONUP<<16)|id, hwnd);
			}
		return 0;

		case WM_RBUTTONDOWN: {
			/* 通知父窗口 */ {
			int id = GetDlgCtrlID(hwnd);
			SendMessage (GetParent(hwnd), WM_COMMAND, (BM_RBUTTONDOWN<<16)|id, hwnd);
			}
		}
		return 0;

		case WM_RBUTTONUP: {
			/* 通知父窗口 */ {
			int id = GetDlgCtrlID(hwnd);
			SendMessage (GetParent(hwnd), WM_COMMAND, (BM_RBUTTONUP<<16)|id, hwnd);
			}
		}
		return 0;

		case WM_PAINT: {
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);
			RECT r;

			GetClientRect (hwnd, &r);

			if (b->bcolor != 0xff00ff)
				i32fillrect(hdc, &r, b->bcolor);
			draw_batton(hwnd, hdc);

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

		case BM_SETRADIUS:
			b->rad = (int)wp;
		return 0;

		case BM_SETMARGIN:
			b->margin = (int)wp;
		return 0;

		case BM_SETTITLEMARGIN:
			b->titlemargin = (int)wp;
		return 0;

		case BM_SETBCOLOR:
			b->bcolor = (DWORD)wp;
		return 0;

		case BM_SETBCOLOR_HOVER:
			b->bcolor_h = (DWORD)wp;
		return 0;

		case BM_SETBCOLOR_PUSHED:
			b->bcolor_p = (DWORD)wp;
		return 0;

		case BM_SETFCOLOR:
			b->fcolor = (DWORD)wp;
		return 0;

		case BM_SETTEXTCOLOR:
			b->textcolor = (DWORD)wp;
		return 0;
	}

	return DefWindowProc(hwnd, msg, wp, lp);
}


void reg_batton ()
{
    WNDCLASSEX wincl;

    /* The Window structure */
    wincl.hInstance = GetModuleHandle(0);
    wincl.lpszClassName = TEXT("batton");
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
