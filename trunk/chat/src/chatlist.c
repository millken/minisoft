/*
 *        chatlist控件: 可分组的好友列表
 */

#include "i32.h"

/* 哈西表大小 */
#define CHATTSIZE 10

#define GROUP_H 20 /* 分组条高度*/
#define BUDDYPIC_H 40 /* 大头像高度 */
#define BUDDY_H 25 /* 小头像和无头像的高度 */

enum SELECT_STATE {
	NONE = 0,
	HOVER,
	PUSHED
};

/* 个人类型 */
typedef struct chatbuddy {
	int uid;

	struct chatgroup *group; /* 所属组 */

	TCHAR *name; /* 名字 */
	TCHAR *note; /* 注释,名字后面的括号 */
	TCHAR *sign; /* 签名 */

	HBITMAP pic; /* 头像 */

	int status; /* 在线状态, 0:不在, 1:在 */

	/* 链表指针 */
	struct chatbuddy *next;

} ChatBuddy;

/* 分组类型 */
typedef struct chatgroup {
	int gid;

	TCHAR *name; /* 组名 */

	struct chatlist *chatlist; /* 所在chatlist */

	ChatBuddy *buddylist;
	int bn; /* 好友列表长度 */

	BOOL fold; /* 是否折叠, FALSE:展开,TRUE:折叠 */

	/* 绘制状态 */
	int select_uid; /* 哪个人被鼠标经过 */
	int select_state; /* 选中状态 0:无, 1:hover, 2:按下 */

	/* 链表指针 */
	struct chatgroup *next;

} ChatGroup;

typedef struct chatlist {
	HWND hwnd;

	ChatGroup *grouplist;  /* 0号组是'未分组' */
	int gn;

	int view; /* 联系人视图选项 0:显示大头像, 1:小头像, 2:无头像 */
	BOOL showsb; /* 是否显示滚动条 */

	int top; /* 绘制起点,相对于client */
	int height; /* 总长度 */

	/* 绘制状态 */
	int select_gid; /* 哪个组被鼠标经过 */
	int select_state; /* 选中状态 0:无, 1:hover, 2:按下 */

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


static int get_viewh (ChatList *cl)
{
	switch (cl->view) {
		case 0:
		return BUDDYPIC_H;
		case 1:
		case 2:
		return BUDDY_H;
	}
	return 0;
}

/* 获得总高度 */
static int get_chatlist_h (ChatList *cl)
{
	int h = 0;
	ChatGroup *g;

	if (!cl) return h;

	for (g = cl->grouplist; g; g = g->next)
		h += g->bn * get_viewh(cl) + GROUP_H;
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
	if (!group) return;

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
	p->group = group;
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

/* 重点!绘制列表 */
static void
draw_chatlist (HWND hwnd, HDC hdc, ChatList *cl)
{
	ChatGroup *g;
	ChatBuddy *b;
	int accHeight = cl->top;

	HFONT hfont = CreateFont (14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		BALTIC_CHARSET, OUT_CHARACTER_PRECIS,
		CLIP_DEFAULT_PRECIS, PROOF_QUALITY,
		 VARIABLE_PITCH|FF_SWISS, TEXT("Verdana"));
	SelectObject (hdc, hfont);
	SetBkMode(hdc, TRANSPARENT);

	int ClientWidth = i32clientw (hwnd);

	for (g = cl->grouplist; g; g = g->next) {
		RECT gr;

		/* 0号组是纯列表 */
		if (g->gid > 0) {
			gr.top = accHeight;
			gr.bottom = gr.top + GROUP_H;
			gr.left = 0;
			gr.right = ClientWidth - 0;
			if (cl->select_gid==g->gid && cl->select_state==HOVER)
				i32fillrect (hdc, &gr, 0x888888);
			else if (cl->select_gid==g->gid && cl->select_state==PUSHED)
				i32fillrect (hdc, &gr, 0x555555);
			else
				i32fillrect (hdc, &gr, 0x33CCFF);

			accHeight = gr.bottom;
		}

		for (b = g->buddylist; b; b = b->next) {
			RECT br;
			br.top = accHeight;
			br.bottom = br.top + get_viewh(cl);
			br.left = 0;
			br.right = ClientWidth - 0;
			if (g->select_uid == b->uid && g->select_state==HOVER)
				i32fillrect (hdc, &br, RGB(237, 243, 250));
			else if (g->select_uid == b->uid && g->select_state==PUSHED)
				i32fillrect (hdc, &br, RGB(232, 238, 245));
			else
				i32fillrect (hdc, &br, 0xffffff);

			if (b->name)
				TextOut(hdc, 50, accHeight+5, b->name, lstrlen(b->name));

			if (b->sign) {
				SetTextColor(hdc, 0x888888);
				TextOut(hdc, 50, accHeight+22, b->sign, lstrlen(b->sign));
			}

			if (b->pic)
				i32bltbmp (hdc, b->pic, 8, 4);

			accHeight = br.bottom;
		}
	}

	DeleteObject(hfont);
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

static void CursorPos (HWND hwnd, POINT *p)
{
	GetCursorPos(p);
	ScreenToClient(hwnd, p);
}

/* 获得鼠标经过的条目(组,人)
 * 返回0,没东西; 返回1,out出来是个ChatGroup; 返回2,是ChatBuddy;
 */
static int feedback (ChatList *cl, POINT *p, ChatGroup **group, ChatBuddy **buddy)
{
	int x, y;
	ChatGroup *g;
	ChatBuddy *b;

	int accHeight = 0;
	int viewh = 0;

	if (!cl || !p) return 0;

	x = p->x;
	y = p->y;

	if (x < 0 || y < 0) return 0;

	viewh = get_viewh (cl);
	for (g = cl->grouplist; g; g = g->next) {
		/* 在组item上 */
		if (g->gid>0 && y < accHeight+GROUP_H) {
			*group = g;
			return 1;
		}
		if (g->gid>0) accHeight += GROUP_H;
		/* 在组里寻找 */
		/* 不在这组 */
		if (y >= accHeight + g->bn*viewh) {
			accHeight += g->bn*viewh;
			continue;
		}
		for (b = g->buddylist; b; b = b->next) {
			if (y < accHeight + viewh) {
				*buddy = b;
				return 2;
			}
			accHeight += viewh;
		}

	}

	return 0;
}

static LRESULT CALLBACK
chatlist_proc (HWND hwnd, UINT message, WPARAM wp, LPARAM lp)
{
	ChatList *cl = get_chatlist(hwnd);

	/* 上次选中的组和人 */
	static ChatGroup *selgroup = NULL;
	static ChatBuddy *selbuddy = NULL;

	switch (message) {
		case WM_CREATE:
			init_chattable ();
			{
			ChatGroup *g;
			ChatBuddy *b;
			cl = new_chatlist (hwnd);
			g = new_chatgroup(cl, 0);
			b = new_chatbuddy(g, 1);
			b->name = TEXT("Cat 못한");
			b->pic = LoadBitmap(GetModuleHandle(0), TEXT("CHAT_THUMB"));
			b->sign = TEXT("只缘身在此山中");
			new_chatbuddy(g, 2);
			b = new_chatbuddy(g, 3);
			b->name = TEXT("sdны");
			new_chatbuddy(g, 4);
			new_chatbuddy(g, 5);
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

		case WM_ERASEBKGND:
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

			switch (LOWORD(wp)) {
				case SB_TOP:
					si.nPos = si.nMin;
				break;

				case SB_BOTTOM:
					si.nPos = si.nMax;
				break;

				case SB_LINEUP:
					si.nPos -= BUDDYPIC_H/2;
				break;

				case SB_LINEDOWN:
					si.nPos += BUDDYPIC_H/2;
				break;

				case SB_PAGEUP:
					si.nPos -= clienth;
				break;

				case SB_PAGEDOWN:
					si.nPos += clienth;
				break;

				case SB_THUMBTRACK:
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

		case WM_MOUSELEAVE:
			printf ("l");
			InvalidateRect(hwnd, NULL, TRUE);
		return 0;

		case WM_LBUTTONUP:
		case WM_MOUSEMOVE: {
			ChatGroup *g;
			ChatBuddy *b;
			POINT p;
			int r;
			CursorPos (hwnd, &p);
			p.y -= cl->top; /* 转化成列表坐标 */
			r = feedback (cl, &p, &g, &b);
			if (r == 1) {
				cl->select_gid = g->gid;
				cl->select_state = HOVER;
				selgroup = g;
				/* 消除上次选中的人 */
				if (selbuddy) {
					selbuddy->group->select_uid = 0;
					selbuddy->group->select_state = 0;
					selbuddy = NULL;
				}
			}
			else if (r == 2) {
				g = b->group;
				g->select_uid = b->uid;
				g->select_state = HOVER;
				selbuddy = b;
				/* 消除上次选中的组 */
				if (selgroup) {
					cl->select_gid = 0;
					cl->select_state = 0;
					selgroup = NULL;
				}
			}
			InvalidateRect(hwnd, NULL, TRUE);
		}
		return 0;

		case WM_LBUTTONDOWN: {
			if (selgroup) {
				cl->select_state = PUSHED;
				InvalidateRect(hwnd, NULL, TRUE);
			}
			else if (selbuddy) {
				selbuddy->group->select_state = PUSHED;
				InvalidateRect(hwnd, NULL, TRUE);
			}
		}
		return 0;

	}

	return DefWindowProc(hwnd, message, wp, lp);
}


static void reg (TCHAR *classname, WNDPROC f)
{
    WNDCLASSEX wincl;

    /* The Window structure */
    wincl.hInstance = GetModuleHandle(0);
    wincl.lpszClassName = classname;
    wincl.lpfnWndProc = f;      /* This function is called by windows */
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

void reg_chatlist ()
{
	reg(TEXT("chatlist"), chatlist_proc);
}
