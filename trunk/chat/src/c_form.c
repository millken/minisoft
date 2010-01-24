/***************************************
 *                FORM
 ***************************************/
#include "i32.h"
#include "c.h"


/* �߿�ߴ� */
static int NCPADDING_LEFT = 4;
static int NCPADDING_RIGHT = 4;
static int NCPADDING_TOP = 20;
static int NCPADDING_BOTTOM = 4;

/* formͼƬ */
static HBITMAP bmphead = 0;
static HBITMAP bmpleft = 0;
static HBITMAP bmpright = 0;
static HBITMAP bmpfoot = 0;
static HBITMAP bmpmin = 0;
static HBITMAP bmpclose = 0;
static HBITMAP bmpico = 0;

/* ���ڲü��� */
static HRGN FormRgn = 0;

/* ��ť�ߴ� */
static RECT CloseRect = {0, 0, 0, 0};
static RECT MinRect = {0, 0, 0, 0};

/* ��ť״̬ */
enum BUTTON_STATE {
	BS_NORMAL,
	BS_HOVER,
	BS_DOWN,
	BS_BLUR
};

/* ͼ��css */
static int IconLeft = 5;
static int IconTop = 6;
/* ����css */
static int TitleLeft = 25;
static int TitleTop = 8;
static DWORD TitleColor = 0xFFFFFF;
/* ��ťcss */
static int BtnMarginLeft = -36; /* NC��ť���ϽǶ�λ */
static int BtnMarginTop = 5;
static int BtnSpace = 2;
/* ����css */
static int RNDH = 4; /* Բ�ǰ뾶 */
static int RNDW = 4;
/* �߿� */
static DWORD FrameColor = 0x000000;


static
void bltBmp (HDC hdc, HBITMAP hbmp, int x, int y)
{
	BITMAP bmp;
	HDC hmem = 0;

	GetObject (hbmp, sizeof(bmp), &bmp);
	hmem = CreateCompatibleDC(hdc);
	SelectObject(hmem, hbmp);

	BitBlt(hdc, x, y, bmp.bmWidth, bmp.bmHeight, hmem, 0, 0, SRCCOPY);/**/
	/* ��Ҫ���ӿ� - msimg32.a */
	//TransparentBlt (hdc, x, y, bmp.bmWidth, bmp.bmHeight, hmem,
	//	0, 0, bmp.bmWidth, bmp.bmHeight, RGB(255, 0, 255));

	DeleteObject(hmem);
}

static
void stretchBmp (HDC hdc, HBITMAP hbmp, int x, int y, int w, int h)
{
	BITMAP bmp;
	HDC hmem = 0;

	GetObject (hbmp, sizeof(bmp), &bmp);
	hmem = CreateCompatibleDC(hdc);
	SelectObject(hmem, hbmp);

	StretchBlt (hdc, x, y, w, h, hmem, 0, 0, bmp.bmWidth, bmp.bmHeight, SRCCOPY);
	//TransparentBlt (hdc, x, y, w, h, hmem, 0, 0, bmp.bmWidth, bmp.bmHeight, RGB(255, 0, 255));
	DeleteObject(hmem);
}

/* ��4��״̬����ƴ�ɵİ�ť��ͼƬ */
static
void bltBtn4 (HDC hdc, HBITMAP hbmp, int x, int y, int index)
{
	BITMAP bmp;
	HDC hmem = 0;
	int tilew = 0;

	GetObject (hbmp, sizeof(bmp), &bmp);
	tilew = bmp.bmWidth / 4;
	hmem = CreateCompatibleDC(hdc);
	SelectObject(hmem, hbmp);

	BitBlt(hdc, x, y, tilew, bmp.bmHeight, hmem, index*tilew, 0, SRCCOPY);

	DeleteObject(hmem);
}

static
void getNcRect (HWND hwnd, RECT *r)
{
	if (r == NULL) return;

	GetClientRect(hwnd, r);
	r->right += NCPADDING_LEFT + NCPADDING_RIGHT;
	r->bottom += NCPADDING_TOP + NCPADDING_BOTTOM;
}

static
void getCloseRect (HWND hwnd, RECT *r)
{
	RECT ncrect;

	getNcRect(hwnd, &ncrect);
	MinRect.left = BtnMarginLeft>=0 ? ncrect.left+BtnMarginLeft : ncrect.right+BtnMarginLeft;
	CloseRect.left = MinRect.left + MinRect.right + BtnSpace;

	CopyRect (r, &CloseRect);
	r->right += r->left;
	r->bottom += r->top;
}

static
void getMinRect (HWND hwnd, RECT *r)
{
	RECT ncrect;

	getNcRect(hwnd, &ncrect);
	MinRect.left = BtnMarginLeft>=0 ? ncrect.left+BtnMarginLeft : ncrect.right+BtnMarginLeft;

	CopyRect (r, &MinRect);
	r->right += r->left;
	r->bottom += r->top;
}

static
void drawMin (HWND hwnd, HDC hdc, int state)
{
	RECT r;

	getMinRect (hwnd, &r);
	bltBtn4 (hdc, bmpmin, r.left, r.top, state);
}

static
void drawClose (HWND hwnd, HDC hdc, int state)
{
	RECT r;

	getCloseRect (hwnd, &r);
	bltBtn4 (hdc, bmpclose, r.left, r.top, state);
}

static
void drawHeadIcon (HDC hdc)
{
	bltBmp (hdc, bmpico, IconLeft, IconTop);
}

static
void drawTitle (HWND hwnd, HDC hdc)
{
	char buf[32];
	HFONT hfont;

	hfont = CreateFont (15, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_CHARACTER_PRECIS,
		CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY,
		FF_MODERN, "Verdana");
	SelectObject (hdc, hfont);
	SetTextColor (hdc, TitleColor);

	GetWindowText(hwnd, buf, sizeof(buf));
	TextOut (hdc, TitleLeft, TitleTop, buf, strlen(buf));

	DeleteObject (hfont);
}

static
void drawFormFrame (HDC hdc)
{
	HBRUSH hbrush = CreateSolidBrush(FrameColor);
	FrameRgn (hdc, FormRgn, hbrush, 1, 1);
	DeleteObject(hbrush);
}

static void ncCursorPos (HWND hwnd, POINT *p)
{
	GetCursorPos(p);
	ScreenToClient(hwnd, p);
	p->x += NCPADDING_LEFT;
	p->y += NCPADDING_TOP;
}

static BOOL ptInClose (HWND hwnd)
{
	POINT p;
	RECT r;

	getCloseRect(hwnd, &r);
	ncCursorPos(hwnd, &p);

	return PtInRect (&r, p);
}

static BOOL ptInMin (HWND hwnd)
{
	POINT p;
	RECT r;

	getMinRect(hwnd, &r);
	ncCursorPos(hwnd, &p);
	return PtInRect (&r, p);
}

static BOOL ptInBody (HWND hwnd)
{
	POINT p;
	RECT r;

	GetCursorPos(&p);
	ScreenToClient(hwnd, &p);
	GetClientRect(hwnd, &r);

	return PtInRect(&r, p);
}

static BOOL ptInBorder (HWND hwnd)
{
	POINT p;
	RECT r;

	getNcRect (hwnd, &r);
	r.left += NCPADDING_LEFT;
	r.right -= NCPADDING_RIGHT;
	r.top += NCPADDING_LEFT; /* ���û�� */
	r.bottom -= NCPADDING_BOTTOM;
	ncCursorPos (hwnd, &p);

	return !PtInRect(&r, p);
}

static
HRGN createFormRgn (HWND hwnd)
{
	RECT r;
	HRGN rgn1, rgn2;
	getNcRect (hwnd, &r);

	rgn1 = CreateRoundRectRgn(0, 0, r.right+1, r.bottom-RNDH, RNDW, RNDH);
	rgn2 = CreateRectRgn (0, RNDH, r.right, r.bottom);
	CombineRgn (rgn1, rgn1, rgn2, RGN_OR);
	DeleteObject(rgn2);

	return rgn1;
}

static LRESULT CALLBACK
form_proc (HWND hwnd, UINT message, WPARAM wp, LPARAM lp)
{
	static BOOL bInClose = FALSE;
	static BOOL bInMin = FALSE;
	static BOOL bCloseDown = FALSE; /*ȷ���Ȱ����˲Ŵ���������keyup*/
	static BOOL bMinDown = FALSE;

    switch (message)                  /* handle the messages */
    {
		case WM_CREATE: {
			BITMAP bmp;
			HINSTANCE hinstance = GetModuleHandle(NULL);
			bmphead = LoadBitmap(hinstance, "FORM_HEAD");
			bmpfoot = LoadBitmap(hinstance, "FORM_FOOT");
			bmpleft = LoadBitmap(hinstance, "FORM_LEFT");
			bmpright = LoadBitmap(hinstance, "FORM_RIGHT");
			/* ��ťͼƬ������4��״̬���� */
			bmpmin = LoadBitmap(hinstance, "FORM_MIN");
			bmpclose = LoadBitmap(hinstance, "FORM_CLOSE");
			bmpico = LoadBitmap(hinstance, "FORM_ICON");

			GetObject (bmphead, sizeof(BITMAP), &bmp);
			NCPADDING_TOP = bmp.bmHeight;
			GetObject (bmpfoot, sizeof(BITMAP), &bmp);
			NCPADDING_BOTTOM = bmp.bmHeight;
			GetObject (bmpleft, sizeof(BITMAP), &bmp);
			NCPADDING_LEFT = bmp.bmWidth;
			GetObject (bmpright, sizeof(BITMAP), &bmp);
			NCPADDING_RIGHT = bmp.bmWidth;
			GetObject (bmpmin, sizeof(BITMAP), &bmp);
			MinRect.bottom = bmp.bmHeight;
			MinRect.right = bmp.bmWidth/4;
			MinRect.top = BtnMarginTop;
			GetObject (bmpclose, sizeof(BITMAP), &bmp);
			CloseRect.bottom = bmp.bmHeight;
			CloseRect.right = bmp.bmWidth/4;
			CloseRect.top = BtnMarginTop;
		}
		return 0;

		case WM_DESTROY: {
			DeleteObject(bmphead);
			DeleteObject(bmpfoot);
			DeleteObject(bmpleft);
			DeleteObject(bmpright);
		}
		break;

		case WM_EXITSIZEMOVE:
		case WM_MOVE:
		case WM_MOVING:
		case WM_SIZING:
		case WM_ACTIVATE:
		case WM_SIZE: {
			static int oldw = 0;
			static int oldh = 0;
			RECT wr;
			int w, h;

			GetWindowRect (hwnd, &wr);
			w = wr.right - wr.left;
			h = wr.bottom - wr.top;

			if (w==oldw && h==oldh)
				return 0;

			if (FormRgn) {
				DeleteObject(FormRgn);
				FormRgn = NULL;
			}
			FormRgn = createFormRgn(hwnd);
			SetWindowRgn (hwnd, FormRgn, TRUE);

			oldw = w;
			oldh = h;
		}
		return 0;

		case WM_NCCALCSIZE: {
			RECT *r = &((NCCALCSIZE_PARAMS *)lp)->rgrc[0];
			r->top += NCPADDING_TOP;
			r->left += NCPADDING_LEFT;
			r->right -= NCPADDING_RIGHT;
			r->bottom -= NCPADDING_BOTTOM;
		}
		return 0;

		case WM_NCPAINT: {
			HDC hdc = GetWindowDC(hwnd);
			RECT r;
			HBITMAP hbmp;
			HDC hmem;

			getNcRect (hwnd, &r);

			hbmp = CreateCompatibleBitmap(hdc, r.right, r.bottom);
			hmem = CreateCompatibleDC(hdc);
			SelectObject(hmem, hbmp);

			SetBkMode (hmem, TRANSPARENT);

			i32fillrect (hmem, &r, 0xff00ff);
			stretchBmp (hmem, bmphead, 0, 0, r.right, NCPADDING_TOP);
			stretchBmp (hmem, bmpfoot, 0, r.bottom-NCPADDING_BOTTOM, r.right, NCPADDING_BOTTOM);
			stretchBmp (hmem, bmpleft, 0, NCPADDING_TOP, NCPADDING_LEFT, r.bottom-NCPADDING_BOTTOM-NCPADDING_TOP);
			stretchBmp (hmem, bmpright, r.right-NCPADDING_RIGHT, NCPADDING_TOP, NCPADDING_RIGHT, r.bottom-NCPADDING_BOTTOM-NCPADDING_TOP);

			drawHeadIcon (hmem);
			drawTitle (hwnd, hmem);

			drawMin (hwnd, hmem, BS_NORMAL);
			drawClose (hwnd, hmem, BS_NORMAL);

			FormRgn = createFormRgn(hwnd);
			drawFormFrame (hmem);

			/* ���ſͻ�����4�黭 */
			BitBlt (hdc, 0, 0, r.right, NCPADDING_TOP,
					hmem, 0, 0, SRCCOPY);
			BitBlt (hdc, 0, r.bottom-NCPADDING_BOTTOM, r.right, NCPADDING_BOTTOM,
					hmem, 0, r.bottom-NCPADDING_BOTTOM, SRCCOPY);
			BitBlt (hdc, 0, NCPADDING_TOP, NCPADDING_LEFT, r.bottom-NCPADDING_BOTTOM-NCPADDING_TOP,
					hmem, 0, NCPADDING_TOP, SRCCOPY);
			BitBlt (hdc, r.right-NCPADDING_RIGHT, NCPADDING_TOP, NCPADDING_RIGHT, r.bottom-NCPADDING_BOTTOM-NCPADDING_TOP,
					hmem, r.right-NCPADDING_RIGHT, NCPADDING_TOP, SRCCOPY);

			DeleteObject(hbmp);
			DeleteObject(hmem);

			ReleaseDC (hwnd, hdc);
		}
		return 0;

		case WM_NCLBUTTONDOWN: {
			HDC hdc = GetWindowDC(hwnd);
			if (ptInMin(hwnd)) {
				drawMin (hwnd, hdc, BS_DOWN);
				bMinDown = TRUE;
				return 0;
			}
			else if (ptInClose(hwnd)) {
				drawClose (hwnd, hdc, BS_DOWN);
				bCloseDown = TRUE;
				return 0;
			}
			ReleaseDC(hwnd, hdc);
		}
		break;

		case WM_NCLBUTTONUP: {
			HDC hdc = GetWindowDC(hwnd);
			if (ptInClose(hwnd)) {
				if (bCloseDown) {
					drawClose (hwnd, hdc, BS_NORMAL);
					SendMessage(hwnd, WM_SYSCOMMAND, SC_CLOSE, 0);
				}
				bCloseDown = FALSE;
				return 0;
			}
			else if (ptInMin(hwnd)) {
				if (bMinDown) {
					drawMin(hwnd, hdc, BS_NORMAL);
					SendMessage(hwnd, WM_SYSCOMMAND, SC_MINIMIZE, 0);
				}
				bMinDown = FALSE;
				return 0;
			}
			ReleaseDC(hwnd, hdc);
		}
		break;


		case WM_MOUSEMOVE:
		case WM_NCMOUSEMOVE: {
			HDC hdc;

			hdc = GetWindowDC(hwnd);
			if (ptInClose(hwnd)) {
				if (!bInClose) {
					drawClose (hwnd, hdc, BS_HOVER);
					bInClose = TRUE;
				}
			}
			else {
				if (bInClose) {
					drawClose (hwnd, hdc, BS_NORMAL);
					bInClose = FALSE;
				}
				bCloseDown = FALSE;
			}

			if (ptInMin(hwnd)) {
				if (!bInMin) {
					drawMin (hwnd, hdc, BS_HOVER);
					bInMin = TRUE;
				}
			}
			else {
				if (bInMin) {
					drawMin (hwnd, hdc, BS_NORMAL);
					bInMin = FALSE;
				}
				bMinDown = FALSE;
			}

			ReleaseDC(hwnd, hdc);
		}
		break;

		case WM_NCHITTEST: {
			if (ptInClose(hwnd))
				return HTCLOSE;
			else if (ptInMin(hwnd))
				return HTMINBUTTON;
			else if (!ptInBody(hwnd) && !ptInBorder(hwnd))
				return HTCAPTION;
		}
		break;

    }
	return DefWindowProc (hwnd, message, wp, lp);
}


static void reg (char *classname, WNDPROC f)
{
    WNDCLASSEX wincl;

    /* The Window structure */
    wincl.hInstance = GetModuleHandle(0);
    wincl.lpszClassName = classname;
    wincl.lpfnWndProc = f;      /* This function is called by windows */
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
    wincl.hbrBackground = (HBRUSH) COLOR_BACKGROUND;

    /* Register the window class, and if it fails quit the program */
    RegisterClassEx (&wincl);
}

void reg_form ()
{
	reg("form", form_proc);
}
