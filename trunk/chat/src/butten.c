/*
 * butten 按钮控件
 */

#include "i32.h"
#include "ctrls.h"

enum {
	NONE = 0,
	HOVER,
	PUSHED
};

enum {
	LEFT = 0,
	RIGHT,
	CENTER
};

#define DEF_MARGIN 0
#define DEF_TITLEMARGIN 4
#define DEF_BCOLOR 0xff00ff
#define DEF_BCOLOR_HOVER 0xff00ff
#define DEF_BCOLOR_PUSHED 0xff00ff
#define DEF_FCOLOR 0xff00ff
#define DEF_RADIUS 0
#define DEF_ICONALIGN LEFT

#define BTSIZE 64

static struct butten {
	HWND hwnd;

	HBITMAP hbmp;
	HFONT hfont; /* 自定义控件貌似要自己做WM_SETFONT */
	int rad; /* 圆角半径 */
	int align; /* 内容对齐 */
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

	struct butten *next;

} *g_buttentable[BTSIZE]; /* 哈系表 */


static void init ()
{
	memset(g_buttentable, 0, sizeof(g_buttentable));
}

static void butten_new (HWND hwnd)
{
	struct butten **lp, *p;

	lp = &g_buttentable[(unsigned)hwnd%BTSIZE];
	while (*lp) {
		p = *lp;
		if (p->hwnd == hwnd)
			return;
		lp = &p->next;
	}

	*lp = (struct butten *)i32malloc(sizeof(struct butten));
	p = *lp;
	memset(p, 0, sizeof(struct butten));
	p->hwnd = hwnd;
	p->bcolor = DEF_BCOLOR;
	p->bcolor_h = DEF_BCOLOR_HOVER;
	p->bcolor_p = DEF_BCOLOR_PUSHED;
	p->fcolor = DEF_FCOLOR;
	p->rad = DEF_RADIUS;
	p->margin = DEF_MARGIN;
	p->titlemargin = DEF_TITLEMARGIN;
	p->iconalign = DEF_ICONALIGN;
	p->align = CENTER;
}

static struct butten *butten_get (HWND hwnd)
{
	struct butten *p;
	unsigned hashcode = (unsigned)hwnd % BTSIZE;

	for (p = g_buttentable[hashcode]; p; p = p->next)
		if (p->hwnd == hwnd)
			return p;
	return NULL;
}

static void butten_del (HWND hwnd)
{
	struct butten **lp, *p;
	unsigned hashcode = (unsigned)hwnd % BTSIZE;

	lp = &g_buttentable[hashcode];
	while ((p = *lp)) {
		struct butten *next = p->next;
		if (p->hwnd == hwnd) {
			i32free(p);
			*lp = next;
			return;
		}
		lp = &next;
	}
}

static void draw_butten (HWND hwnd, HDC hdc)
{
	int buttenw = 0, buttenh;
	TCHAR title[32];
	int titlelen = GetWindowText(hwnd, title, -1);
	int titlew, titleh, iconw, left = 0;
	BITMAP bmp;
	struct butten *b;
	int titlex, iconx;
	int contentw = 0; /* 图标+文字的宽度 */

	buttenh = i32clienth(hwnd);
	buttenw = i32clientw(hwnd);

	b = butten_get(hwnd);
	if (!b) return;

	SelectObject(hdc, b->hfont);

	/* 计算按钮宽度 */ {
		SIZE tsize;
		GetTextExtentPoint32 (hdc, title, titlelen, &tsize);
		titlew = tsize.cx;
		titleh = tsize.cy;
		contentw += titlew;
		if (b->hbmp) {
			GetObject(b->hbmp, sizeof(bmp), &bmp);
			iconw = bmp.bmWidth;
			contentw += iconw;
			if (titlew>0)
				contentw += b->titlemargin;
		}
		contentw += 2*b->margin;
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
		if ((b->status==HOVER&&b->bcolor_h!=0xff00ff) ||
			(b->status==PUSHED&&b->bcolor_p!=0xff00ff))
		RoundRect(hdc, inner, inner, buttenw-inner, buttenh-inner, b->rad, b->rad);
		DeleteObject(hbrush);
		DeleteObject(hpen);
	}

	/* 内容对齐 */
	if (b->align == CENTER)
		left = (buttenw-contentw)/2; /* 居中 */
	else if (b->align == LEFT)
		left = b->margin;
	else
		left = buttenw - contentw;

	/* 图标对齐 */
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

	/* 画标题 */
	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, b->textcolor);
	TextOut (hdc, titlex, (buttenh-titleh)/2, title, titlelen);

	/* 画图标 */
	if (b->hbmp) {
		i32blt(hdc, b->hbmp, iconx, (buttenh-bmp.bmHeight)/2);
	}
}

static LRESULT CALLBACK
win_proc (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	struct butten *b = butten_get(hwnd);

	switch (msg) {
		case WM_CREATE:
			butten_new(hwnd);
		return 0;

		case WM_DESTROY:
			butten_del(hwnd);
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
			SendMessage (GetParent(hwnd), WM_COMMAND, (BM_LBUTTONDOWN<<16)|id, (LPARAM)hwnd);
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
			SendMessage (GetParent(hwnd), WM_COMMAND, (BM_LBUTTONUP<<16)|id, (LPARAM)hwnd);
			}
		return 0;

		case WM_RBUTTONDOWN: {
			/* 通知父窗口 */ {
			int id = GetDlgCtrlID(hwnd);
			SendMessage (GetParent(hwnd), WM_COMMAND, (BM_RBUTTONDOWN<<16)|id, (LPARAM)hwnd);
			}
		}
		return 0;

		case WM_RBUTTONUP: {
			/* 通知父窗口 */ {
			int id = GetDlgCtrlID(hwnd);
			SendMessage (GetParent(hwnd), WM_COMMAND, (BM_RBUTTONUP<<16)|id, (LPARAM)hwnd);
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
			draw_butten(hwnd, hdc);

			EndPaint(hwnd, &ps);
		}
		return 0;


		/* 输入接口 */

		case BM_SETALIGN:
			b->align = (int)wp;
		return 0;

		case BM_SETICON: {
			if (b->hbmp)
				DeleteObject(b->hbmp);
			b->hbmp = (HBITMAP)wp;
		}
		return 0;

		case BM_SETICONALIGN:
			b->iconalign = RIGHT;
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


void reg_butten ()
{
    WNDCLASSEX wincl;

    /* The Window structure */
    wincl.hInstance = GetModuleHandle(0);
    wincl.lpszClassName = TEXT("butten");
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
