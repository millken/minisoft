#include "myctl.h"

/**
 * 分组列表
 */

/* 配置 */
enum glist {
	GLISTMAXN = 10,
	GROUPMAXN = 30,
	ITEMMAXN = 500,

	/* 列表项高度 */
	GROUPLISTH = 18, /* 分组条高度 */
	ITEMLISTH = 24, /* 列表条高度 */
};

static
struct grouplist {
	HWND hwnd;
	Group *groups;
	int gn;
	Item *items;
	int in;
} Glist[GLISTMAXN];

static void init_grouplist ()
{
	static int inited = FALSE;
	if (inited)
		return;
	memset (&Glist, 0, sizeof(Glist));
	inited = TRUE;
}

static void
new_grouplist (HWND hwnd)
{
	int i;

	init_grouplist();

	for (i = 0; i < GLISTMAXN; i++)
		if (Glist[i].hwnd == NULL) {
			Glist[i].hwnd = hwnd;
			Glist[i].groups = (struct group *)malloc(sizeof(struct group)*GROUPMAXN);
			Glist[i].gn = 0;
			Glist[i].items = (struct item *)malloc(sizeof(struct item)*ITEMMAXN);
			Glist[i].in = 0;
			//printf ("newgroup, %d %d %d\n", i, Glist[i].gn, Glist[i].in);
			return;
		}
	exit(1001); /* 必须成功 */
}


static
struct grouplist *get_grouplist (HWND hwnd)
{
	int i;

	for (i = 0; i < GLISTMAXN; i++)
		if (Glist[i].hwnd == hwnd)
			return &Glist[i];
	return NULL;
}


static void del_grouplist (HWND hwnd)
{
	struct grouplist *glist;

	glist = get_grouplist (hwnd);
	free(glist->groups);
	free(glist->items);
	memset(glist, 0, sizeof(struct grouplist));
}

static Group *
get_group (struct grouplist *glist, int gid)
{
	int i;

	if (glist == NULL) {
		return NULL;
	}

	for (i = 0; i < glist->gn; i++)
		if (glist->groups[i].gid == gid)
			return &glist->groups[i];

	return NULL;
}


static int
new_group (struct grouplist *glist, Group *group)
{
	Group *newgroup;

	if (!group)
		return 0;
	if (get_group(glist, group->gid))
		return 0;
	if (glist->gn >= GROUPMAXN)
		return 0;
	newgroup = &glist->groups[glist->gn++];
	newgroup->gid = group->gid;
	newgroup->name = (char *)i32malloc(strlen(group->name)+1);
	strcpy(newgroup->name, group->name);
	//printf ("%d %s %d\n", newgroup->gid, newgroup->name, glist->gn);
	return 1;
}

static int
del_group (struct grouplist *glist, int gid)
{
	Group *groups;
	int i;

	if (glist->gn < 1)
		return 0;

	groups = glist->groups;
	for (i = 0; i < glist->gn; i++)
		if (groups[i].gid == gid)
			break;
	if (i >= glist->gn)  /* 没找着 */
		return 0;

	free (groups[i].name);

	for (; i < glist->gn-1; i++)
		memmove (groups+i, groups+i+1, sizeof(Group));

	glist->gn--;
	memset (&groups[glist->gn], 0, sizeof(Group));
	return 1;
}

static int
rename_group (struct grouplist *glist, int gid, char *name)
{
	Group *g = get_group(glist, gid);
	if (g == NULL)
		return 0;

	if (g->name) {
		free(g->name);
		if (name) {
			g->name = (char *)i32malloc(strlen(name)+1);
			strcpy(g->name, name);
		}
		else
			g->name = name;
	}

	return 1;
	//printf ("newname: %s\n", g->name);
}

static int
new_item (struct grouplist *glist, Item *it)
{
	Item *newit;

	if (glist->in >= ITEMMAXN)
		return 0;
	newit = &glist->items[glist->in++];
	memcpy (newit, it, sizeof(Item));
	if (it->title) {
		newit->title = (char *)i32malloc (strlen(it->title)+1);
		strcpy (newit->title, it->title);
	}
	if (it->title2) {
		newit->title2 = (char *)i32malloc (strlen(it->title2)+1);
		strcpy (newit->title2, it->title2);
	}
	if (it->title3) {
		newit->title3 = (char *)i32malloc (strlen(it->title3)+1);
		strcpy (newit->title3, it->title3);
	}

	//
	int i;
	for (i = 0; i < glist->in; i++) {
		Item *ti = &glist->items[i];
		printf ("{%d, %s}, ", ti->id, ti->title);
	}
	printf ("n:%d\n\n", glist->in);

	return 1;
}

static int
del_item (struct grouplist *glist, int id)
{
	Item *items;
	int i;

	if (glist == NULL)
		return 0;
	items = glist->items;
	for (i = 0; i < glist->in; i++)
		if (items[i].id == id)
			break;
	if (i >= glist->in)
		return 0;

	if (items[i].title) free (items[i].title);
	if (items[i].title2) free (items[i].title2);
	if (items[i].title3) free (items[i].title3);

	for (; i < glist->in-1; i++)
		memmove (&items[i], &items[i+1], sizeof(Item));
	glist->in--;
	memset (&items[glist->in], 0, sizeof(Item));

	//
	for (i = 0; i < glist->in; i++) {
		Item *ti = &glist->items[i];
		printf ("{%d, %s}, ", ti->id, ti->title);
	}
	printf ("n:%d\n\n", glist->in);

	return 1;
}

static void
get_item (struct grouplist *glist, int id, Item *it)
{
	int i;

	if (glist == NULL || it == NULL)
		return;

	for (i = 0; i < glist->in; i++)
		if (glist->items[i].id == id) {
			memmove (it, &glist->items[i], sizeof(Item));
			break;
		}
}

static int
set_item_gid (struct grouplist *glist, int id, int gid)
{
	int i;

	if (glist == NULL)
		return 0;

	for (i = 0; i < glist->in; i++)
		if (glist->items[i].id == id) {
			glist->items[i].gid = gid;
			return 1;
		}
	return 0;
}

void framerect (HDC hdc, RECT *r, DWORD col)
{
	HBRUSH hbrush = CreateSolidBrush(col);
	FillRect(hdc, r, hbrush);
	DeleteObject(hbrush);
}

/* 绘制列表 */
static void
render_grouplist (HDC hdc, struct grouplist *glist)
{
	int i, j;
	int height = 1;
	RECT r;
	HWND hwnd;

	int W;
	HFONT hfont;

	hfont = CreateFont (15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_CHARACTER_PRECIS,
		CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY,
		FF_MODERN, "Verdana");
	SelectObject (hdc, hfont);

	SetBkMode (hdc, TRANSPARENT);
	SetTextColor (hdc, 0x00ffffff);

	hwnd = WindowFromDC(hdc);
	GetClientRect (hwnd, &r);
	W = r.right;

	for (i = 0; i < glist->gn; i++) {
		Group *group = &glist->groups[i];
		int gid = group->gid;

		r.top = height;
		r.bottom = r.top + GROUPLISTH;
		r.left = 1;
		r.right = W - 1;

		framerect(hdc, &r, 0x000000);
		height = r.bottom+1;

		if (group->name)
			TextOut(hdc, r.left+2, r.top+2, group->name, strlen(group->name));

		for (j = 0; j < glist->in; j++) {
			Item *item = &glist->items[j];
			if (item->gid == gid) {
				r.top = height;
				r.bottom = r.top + ITEMLISTH;
				r.left = 1;
				r.right = W - 1;
				framerect (hdc, &r, 0x00220088);
				if (item->title)
					TextOut (hdc, r.right-40, r.top+4, item->title, strlen(item->title));
				height = r.bottom + 1;
			}
		}
	}

	DeleteObject (hfont);
}

static void cursorPos (HWND hwnd, POINT *p)
{
	GetCursorPos(p);
	ScreenToClient(hwnd, p);
}

static LRESULT CALLBACK
grouplist_proc (HWND hwnd, UINT message, WPARAM wp, LPARAM lp)
{
	struct grouplist *glist;
	glist = get_grouplist(hwnd);

    switch (message)                  /* handle the messages */
    {
		case WM_CREATE:
			new_grouplist (hwnd);
		break;

		case WM_DESTROY:
			del_grouplist (hwnd);
		break;

		case WM_PAINT: {
			PAINTSTRUCT ps;
			HDC hdc;
			RECT r = {10, 10, 40, 40};
			hdc = BeginPaint(hwnd, &ps);
			render_grouplist(hdc, glist);
			EndPaint(hwnd, &ps);
		}
		return 0;

		case WM_VSCROLL: {
			switch (LOWORD(wp)) {
				case SB_LINEUP:
					ScrollWindow(hwnd, 0, 6, NULL, NULL);
					printf ("vup\n");
				break;
				case SB_LINEDOWN:
					ScrollWindow(hwnd, 0, -6, NULL, NULL);
					printf ("vdown\n");
				break;
			};
			//InvalidateRect(hwnd, NULL, TRUE);
		}
		return 0;

		case WM_LBUTTONDOWN: {
			POINT p;
			cursorPos (hwnd, &p);
			printf ("cur: %d %d\n", p.x, p.y);
		}
		return 0;


		/* 命令接口 */
		case GLM_ADDGROUP:
			return new_group(glist, (Group *)wp);

		case GLM_DELGROUP:
			return del_group(glist, (int)wp);

		case GLM_RENAMEGROUP:
			return rename_group(glist, (int)wp, (char *)lp);

		case GLM_ADDITEM:
			return new_item(glist, (Item *)wp);

		case GLM_DELITEM:
			return del_item(glist, (int)wp);

		case GLM_GETITEM:
			get_item (glist, (int)wp, (Item *)lp);
		return 0;

		case GLM_SETITEM_GID: {
			int r = set_item_gid (glist, (int)wp, (int)lp);
			InvalidateRect(hwnd, NULL, TRUE); /* 立即重绘,UpdateWindow不管用 */
			return r;
		}
    }
	return DefWindowProc (hwnd, message, wp, lp);
}









/**
 * FORM
 */
/* 边框尺寸 */
static int NCPADDING_LEFT = 4;
static int NCPADDING_RIGHT = 4;
static int NCPADDING_TOP = 20;
static int NCPADDING_BOTTOM = 4;

/* form图片 */
static HBITMAP bmphead = 0;
static HBITMAP bmpleft = 0;
static HBITMAP bmpright = 0;
static HBITMAP bmpfoot = 0;
static HBITMAP bmpmin = 0;
static HBITMAP bmpclose = 0;
static HBITMAP bmpico = 0;

/* 窗口裁减区 */
static HRGN FormRgn = 0;

/* 按钮尺寸 */
static RECT CloseRect = {0, 0, 0, 0};
static RECT MinRect = {0, 0, 0, 0};

/* 按钮状态 */
enum BUTTON_STATE {
	BS_NORMAL,
	BS_HOVER,
	BS_DOWN,
	BS_BLUR
};

/* 图标css */
static int IconLeft = 5;
static int IconTop = 4;
/* 标题css */
static int TitleLeft = 23;
static int TitleTop = 5;
static DWORD TitleColor = 0x408080;
/* 按钮css */
static int BtnMarginLeft = -36; //NC按钮左上角定位
static int BtnMarginTop = 5;
static int BtnSpace = 2;
/* 窗口css */
static int RNDH = 0; /* 圆角半径 */
static int RNDW = 1;
static DWORD FrameColor = 0x00000000; /* 边框颜色 */


static
void bltBmp (HDC hdc, HBITMAP hbmp, int x, int y)
{
	BITMAP bmp;
	HDC hmem = 0;

	GetObject (hbmp, sizeof(bmp), &bmp);
	hmem = CreateCompatibleDC(hdc);
	SelectObject(hmem, hbmp);

	//BitBlt(hdc, x, y, bmp.bmWidth, bmp.bmHeight, hmem, 0, 0, SRCCOPY);
	/* 需要连接库 - msimg32.a */
	TransparentBlt (hdc, x, y, bmp.bmWidth, bmp.bmHeight, hmem,
		0, 0, bmp.bmWidth, bmp.bmHeight, RGB(255, 0, 255));

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

/* 画4个状态横向拼成的按钮类图片 */
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
void drawMin (HDC hdc, int state)
{
	RECT r;

	getMinRect (WindowFromDC(hdc), &r);
	bltBtn4 (hdc, bmpmin, r.left, r.top, state);
}

static
void drawClose (HDC hdc, int state)
{
	RECT r;

	getCloseRect (WindowFromDC(hdc), &r);
	bltBtn4 (hdc, bmpclose, r.left, r.top, state);
}

static
void drawHeadIcon (HDC hdc)
{
	bltBmp (hdc, bmpico, IconLeft, IconTop);
}

static
void drawTitle (HDC hdc)
{
	char buf[32];
	HFONT hfont;

	hfont = CreateFont (15, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_CHARACTER_PRECIS,
		CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY,
		FF_MODERN, "Verdana");
	SelectObject (hdc, hfont);
	SetTextColor (hdc, TitleColor);

	GetWindowText(WindowFromDC(hdc), buf, sizeof(buf));
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
	r.top += NCPADDING_LEFT; /* 这个没错 */
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
	static BOOL bCloseDown = FALSE; //确保先按下了才触发真正的keyup
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
			/* 按钮图片必须是4种状态横排 */
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

		case WM_ACTIVATE:
		case WM_SIZE: {
			if (FormRgn) {
				DeleteObject(FormRgn);
				FormRgn = NULL;
			}
			FormRgn = createFormRgn(hwnd);
			SetWindowRgn (hwnd, FormRgn, TRUE);
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

			SetBkMode (hdc, TRANSPARENT);

			getNcRect (hwnd, &r);
			stretchBmp (hdc, bmphead, 0, 0, r.right, NCPADDING_TOP);
			stretchBmp (hdc, bmpfoot, 0, r.bottom-NCPADDING_BOTTOM, r.right, NCPADDING_BOTTOM);
			stretchBmp (hdc, bmpleft, 0, NCPADDING_TOP, NCPADDING_LEFT, r.bottom-NCPADDING_BOTTOM-NCPADDING_TOP);
			stretchBmp (hdc, bmpright, r.right-NCPADDING_RIGHT, NCPADDING_TOP, NCPADDING_RIGHT, r.bottom-NCPADDING_BOTTOM-NCPADDING_TOP);

			drawHeadIcon (hdc);
			drawTitle (hdc);

			drawMin (hdc, BS_NORMAL);
			drawClose (hdc, BS_NORMAL);

			drawFormFrame (hdc);

			ReleaseDC (hwnd, hdc);
		}
		return 0;

		case WM_NCLBUTTONDOWN: {
			HDC hdc = GetWindowDC(hwnd);
			if (ptInMin(hwnd)) {
				drawMin (hdc, BS_DOWN);
				bMinDown = TRUE;
				return 0;
			}
			else if (ptInClose(hwnd)) {
				drawClose (hdc, BS_DOWN);
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
					drawClose (hdc, BS_NORMAL);
					SendMessage(hwnd, WM_SYSCOMMAND, SC_CLOSE, 0);
				}
				bCloseDown = FALSE;
				return 0;
			}
			else if (ptInMin(hwnd)) {
				if (bMinDown) {
					drawMin(hdc, BS_NORMAL);
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
					drawClose (hdc, BS_HOVER);
					bInClose = TRUE;
				}
			}
			else {
				if (bInClose) {
					drawClose (hdc, BS_NORMAL);
					bInClose = FALSE;
				}
				bCloseDown = FALSE;
			}

			if (ptInMin(hwnd)) {
				if (!bInMin) {
					drawMin (hdc, BS_HOVER);
					bInMin = TRUE;
				}
			}
			else {
				if (bInMin) {
					drawMin (hdc, BS_NORMAL);
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









/**
 * REGISTER CONTROLS NAME
 */
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
    wincl.hbrBackground = (HBRUSH) COLOR_BTNHILIGHT;

    /* Register the window class, and if it fails quit the program */
    RegisterClassEx (&wincl);
}

void reg_myctl ()
{
	reg("grouplist", grouplist_proc);
	reg("form", form_proc);
}
