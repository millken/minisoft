#include "myctl.h"

/****************************************
 *            可分组好友列表
 ****************************************/
/* chattable大小 */
#define CHATTSIZE 10

#define GROUP_H 20 /* 分组高度 */
#define BUDDYPIC_H 35 /* 带头像的好友高度 */
#define BUDDY_H 25 /* 不带头像的高度 */


/* 个人类型 */
typedef struct chatbuddy {
	int uid;

	struct chatgroup *chatgroup; /* 所属组 */

	char *name; /* 名字 */
	char *note; /* 注释,名字后面的括号 */
	char *sign; /* 签名 */

	HBITMAP pic; /* 头像 */

	int status; /* 在线状态, 1:在,0:不在 */

	/* 链表指针 */
	struct chatbuddy *next;

} ChatBuddy;

/* 分组类型 */
typedef struct chatgroup {
	int gid;

	char *name; /* 组名 */

	struct chatlist *chatlist; /* 所在chatlist */

	ChatBuddy *buddylist;
	int bn; /* 好友列表长度 */

	BOOL fold; /* 是否折叠, FALSE:展开,TRUE:折叠 */

	/* 绘制状态 */
	int hoverid; /* 哪个人被鼠标经过 */

	/* 链表指针 */
	struct chatgroup *next;

} ChatGroup;

typedef struct chatlist {
	HWND hwnd;

	ChatGroup *grouplist;  /* 0号组是'未分组' */
	int gn;

	BOOL showpic; /* 是否显示头像 */
	BOOL showsb; /* 是否显示滚动条 */

	int top; /* 绘制起点,相对于client */
	int height; /* 总长度 */

	/* 哈希表开链用 */
	struct chatlist *hashnext;

} ChatList;

/* hwnd -> chatlist的hash表: chattable */
ChatList *g_chattable[CHATTSIZE];


static void init_chattable ()
{
	static BOOL did = FALSE;

	if (!did) {
		memset (g_chattable, 0, sizeof(g_chattable));

		did = TRUE;
	}
}

/* 获得总高度 */
static int get_chatlist_h (ChatList *cl)
{
	int h = 0;
	ChatGroup *g;
	ChatBuddy *b;

	if (!cl) return h;

	for (g = cl->grouplist; g; g = g->next)
		h += g->bn * (cl->showpic?BUDDYPIC_H:BUDDY_H) + GROUP_H;
	cl->height = h;

	return h;
}

static void free_chatbuddy (ChatBuddy *buddy)
{
	if (!buddy) return;

	if (buddy->name)
		i32free(buddy->name);
	if (buddy->note)
		i32free(buddy->note);
	if (buddy->sign)
		i32free(buddy->sign);
	i32free(buddy);
}

static void free_buddylist (ChatBuddy *bl)
{
	ChatBuddy *p, *next;

	p = bl;
	while (p) {
		next = p->next;
		free_chatbuddy (p);
		p = next;
	}
}

static void free_chatgroup (ChatGroup *group)
{
	if (!group) return NULL;

	if (group->name)
		i32free(group->name);
	free_buddylist(group->buddylist);
	i32free(group);
}

static void free_grouplist (ChatList *cl)
{
	ChatGroup *p, *next;

	p = cl->grouplist;
	while (p) {
		next = p->next;
		free_chatgroup(p);
		p = next;
	}
	cl->grouplist = NULL;
	cl->gn = 0;
}

/* 分配新分组 */
static ChatGroup *new_chatgroup (ChatList *cl, int gid)
{
	ChatGroup **lp, *p;

	if (!cl) return NULL;

	lp = &cl->grouplist;
	while (*lp) {
		p = *lp;
		if (p->gid == gid)
			return NULL;
		lp = &p->next;
	}
	*lp = (ChatGroup *)i32malloc(sizeof(ChatGroup));
	p = *lp;
	memset (p, 0, sizeof(ChatGroup));
	p->gid = gid;
	p->chatlist = cl;

	cl->gn++;
	get_chatlist_h(cl);

	return p;
}

static ChatGroup *get_chatgroup (ChatList *cl, int gid)
{
	ChatGroup *p;

	if (!cl) return NULL;

	p = cl->grouplist;
	while (p) {
		if (p->gid == gid)
			break;
		p = p->next;
	}
	return p;
}


/* 从链表里删除分组 */
static void del_chatgroup (ChatList *cl, int gid)
{
	ChatGroup **lp, *p, *next;

	lp = &cl->grouplist;
	while (*lp) {
		p = *lp;
		if (p->gid == gid) {
			next = p->next;
			free_chatgroup (p);
			*lp = next;
			cl->gn--;
			get_chatlist_h(cl);
			return;
		}
		lp = &p->next;
	}
}

/* 给组里分配成员 */
static ChatBuddy *new_chatbuddy (ChatGroup *group, int uid)
{
	ChatBuddy **lp, *p;

	if (!group) return NULL;

	lp = &group->buddylist;
	while (*lp) {
		p = *lp;
		if (p->uid == uid)
			return NULL;
		lp = &p->next;
	}

	*lp = (ChatBuddy *)malloc(sizeof(ChatBuddy));
	p = *lp;
	memset (p, 0, sizeof(ChatBuddy));
	p->uid = uid;
	p->chatgroup = group;
	group->bn++;

	get_chatlist_h (group->chatlist);

	return p;
}

static ChatBuddy *get_chatbuddy (ChatGroup *group, int uid)
{
	ChatBuddy *p;

	if (!group) return NULL;

	p = group->buddylist;
	while (p) {
		if (p->uid == uid)
			break;
		p = p->next;
	}
	return p;
}

/* 从链表中删除好友 */
static void del_chatbuddy (ChatGroup *group, int uid)
{
	ChatBuddy **lp, *p, *next;

	if (!group) return;

	lp = &group->buddylist;
	while (*lp) {
		p = *lp;
		if (p->uid == uid) {
			next = p->next;
			free_chatbuddy (p);
			*lp = next;
			group->bn--;
			get_chatlist_h(group->chatlist);
			return;
		}
		lp = &p->next;
	}

}

static ChatList *new_chatlist (HWND hwnd)
{
	ChatList **lp, *p;

	if (!hwnd) return NULL;

	lp = &g_chattable[(unsigned)hwnd % CHATTSIZE];
	while (*lp) {
		p = *lp;
		if (hwnd == p->hwnd)
			return p;
		lp = &p->hashnext;
	}

	*lp = (ChatList *)malloc(sizeof(ChatList));
	p = *lp;
	memset(p, 0, sizeof(ChatList));
	p->hwnd = hwnd;
	p->showpic = TRUE;
	return p;
}

static ChatList *get_chatlist (HWND hwnd)
{
	ChatList *p;

	if (hwnd == 0) return NULL;

	p = g_chattable[(unsigned)hwnd % CHATTSIZE];
	while (p) {
		if (p->hwnd == hwnd)
			break;
		p = p->hashnext;
	}
	return p;
}

static void del_chatlist (HWND hwnd)
{
	ChatList **lp, *p, *next;

	if (!hwnd) return;

	lp = &g_chattable[(unsigned)hwnd % CHATTSIZE];
	while (*lp) {
		p = *lp;
		if (p->hwnd == hwnd) {
			next = p->hashnext;
			free_grouplist (p);
			i32free(p);
			*lp = next;
			return;
		}
		lp = &p->hashnext;
	}

}

static void
draw_chatlist (HWND hwnd, HDC hdc, ChatList *cl)
{
	ChatGroup *g;
	ChatBuddy *b;
	int accHeight = cl->top;

	int ClientWidth = i32clientw (hwnd);

	for (g = cl->grouplist; g; g = g->next) {
		RECT gr;
		gr.top = accHeight;
		gr.bottom = gr.top + GROUP_H;
		gr.left = 1;
		gr.right = ClientWidth - 1;
		i32fillrect (hdc, &gr, 0x000000);

		accHeight = gr.bottom;

		for (b = g->buddylist; b; b = b->next) {
			RECT br;
			br.top = accHeight;
			br.bottom = br.top + (cl->showpic?BUDDYPIC_H:BUDDY_H);
			br.left = 1;
			br.right = ClientWidth - 1;
			i32framerect (hdc, &br, 0x000000);

			accHeight = br.bottom;
		}
	}

}

static void
chatlist_scroll (ChatList *cl, int dy)
{
	int ClientHeight;

	if (!cl) return;

	ClientHeight = i32clienth(cl->hwnd);

	cl->top += dy;

	if (cl->top > 0 || cl->height <= ClientHeight)
		cl->top = 0;
	else if (cl->top + cl->height < ClientHeight)
		cl->top = ClientHeight - cl->height;
}

/* 重新检查滚动条状态 */
static void
chatlist_check_scrollbar (ChatList *cl)
{
	int clienth;
	SCROLLINFO si;

	if (!cl) return;

	si.cbSize = sizeof(si);
	si.fMask = SIF_RANGE|SIF_PAGE;
	si.nMin = 0;

	clienth = i32clienth(cl->hwnd);
	if (cl->height > clienth) {
		cl->showsb = TRUE;
		si.nMax = cl->height;
	}
	else {
		cl->showsb = FALSE;
		si.nMax = 0; /* 隐藏滚动条 */
	}

	if (si.nMax > 0)
		si.nPage = max(clienth, 1);
	else
		si.nPage = 0;
	SetScrollInfo(cl->hwnd, SB_VERT, &si, TRUE);

	chatlist_scroll (cl, 0);
}

static LRESULT CALLBACK
chatlist_proc (HWND hwnd, UINT message, WPARAM wp, LPARAM lp)
{
	ChatList *cl = get_chatlist(hwnd);

	switch (message) {
		case WM_CREATE:
			init_chattable ();
			{
			ChatGroup *g;
			int h;
			cl = new_chatlist (hwnd);
			g = new_chatgroup(cl, 1);
			new_chatbuddy(g, 1);
			new_chatbuddy(g, 2);
			new_chatbuddy(g, 3);
			new_chatbuddy(g, 4);
			new_chatbuddy(g, 5);
			g = new_chatgroup(cl, 2);
			new_chatbuddy(g, 6);
			new_chatbuddy(g, 7);
			new_chatbuddy(g, 8);
			new_chatbuddy(g, 9);
			new_chatbuddy(g, 10);
			}
		break;

		case WM_ERASEBKGND:
		return 0;

		case WM_PAINT: {
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);
			HDC hmem;
			HBITMAP hbmp;
			RECT r;

			GetClientRect (hwnd, &r);

			hbmp = CreateCompatibleBitmap (hdc, r.right, r.bottom);
			hmem = CreateCompatibleDC (hdc);
			SelectObject (hmem, hbmp);

			{
				RECT tr = {0, 0, r.right, r.bottom};
				i32fillrect (hmem, &tr, 0xffffff);
			}
			draw_chatlist (hwnd, hmem, cl);

			BitBlt (hdc, 0, 0, r.right, r.bottom, hmem, 0, 0, SRCCOPY);

			DeleteObject(hbmp);
			DeleteObject(hmem);

			EndPaint(hwnd, &ps);
		}
		return 0;

		case WM_SIZE:
			chatlist_check_scrollbar(cl);
		return 0;

		case WM_VSCROLL: {
			SCROLLINFO si;
			int clienth = i32clienth(hwnd);

			si.cbSize = sizeof(si);
			si.fMask = SIF_ALL;
			GetScrollInfo (hwnd, SB_VERT, &si);
			printf ("trackpos: %d, pos: %d, page: %d\n", si.nTrackPos, si.nPos, si.nPage);



			switch (LOWORD(wp)) {
				case SB_TOP:
					printf ("SB_TOP\n");
					si.nPos = si.nMin;
				break;

				case SB_BOTTOM:
					printf ("SB_BOTTOM\n");
					si.nPos = si.nMax;
				break;

				case SB_LINEUP:
					printf ("SB_LINEUP\n");
					si.nPos -= BUDDYPIC_H/2;
				break;

				case SB_LINEDOWN:
					printf ("SB_LINEDOWN\n");
					si.nPos += BUDDYPIC_H/2;
				break;

				case SB_PAGEUP:
					printf ("SB_PAGEDOWN\n");
					si.nPos -= clienth;
				break;

				case SB_PAGEDOWN:
					printf ("SB_PAGEUP\n");
					si.nPos += clienth;
				break;

				case SB_THUMBTRACK:
					printf ("SB_THUMBTRACK\n");
					si.nPos = si.nTrackPos;
				break;
			}

			si.fMask = SIF_POS;
			SetScrollInfo (hwnd, SB_VERT, &si, TRUE);

			cl->top = -si.nPos;
			chatlist_check_scrollbar(cl);
			InvalidateRect(hwnd, NULL, TRUE);
		}
		return 0;

		case CLM_SCROLL:
			chatlist_scroll(cl, (int)wp);
		return 0;

	}

	return DefWindowProc(hwnd, message, wp, lp);
}










/***************************************
 *                FORM
 ***************************************/
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
static DWORD TitleColor = 0xFFFFFF;
/* 按钮css */
static int BtnMarginLeft = -36; /* NC按钮左上角定位 */
static int BtnMarginTop = 5;
static int BtnSpace = 2;
/* 窗口css */
static int RNDH = 4; /* 圆角半径 */
static int RNDW = 4;
/* 边框 */
static DWORD FrameColor = 0x000000;


static
void bltBmp (HDC hdc, HBITMAP hbmp, int x, int y)
{
	BITMAP bmp;
	HDC hmem = 0;

	GetObject (hbmp, sizeof(bmp), &bmp);
	hmem = CreateCompatibleDC(hdc);
	SelectObject(hmem, hbmp);

	/*BitBlt(hdc, x, y, bmp.bmWidth, bmp.bmHeight, hmem, 0, 0, SRCCOPY);*/
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
	static BOOL bCloseDown = FALSE; /*确保先按下了才触发真正的keyup*/
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

		case WM_ERASEBKGND:
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

			//BitBlt (hdc, 0, 0, r.right, r.bottom, hmem, 0, 0, SRCCOPY);
			TransparentBlt (hdc, 0, 0, r.right, r.bottom, hmem,
						0, 0, r.right, r.bottom, 0xff00ff);

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
    wincl.hbrBackground = (HBRUSH) COLOR_BACKGROUND;

    /* Register the window class, and if it fails quit the program */
    RegisterClassEx (&wincl);
}

void reg_myctl ()
{
	reg("chatlist", chatlist_proc);
	reg("form", form_proc);
}
