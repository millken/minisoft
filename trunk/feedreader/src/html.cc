/*
 * 附加的html浏览控件
 */

#include <ctype.h>
#include "i32.h"
#include <commdlg.h>
#include "htmlayout/htmlayout.h"
//#include "htmlayout/behaviors/notifications.h" // hyperlink behavior notfication is here
#include "htmlayout/behaviors/notifications.h" // hyperlink behavior notfication is here

#include "html.h"
#include "xml.h"
#include "db.h"



static CALLBACK LRESULT win_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	BOOL bhandled;
	LRESULT result = HTMLayoutProcND(hwnd,msg,wp,lp, &bhandled);
	if (bhandled)
		return result;

	switch (msg) {
		case WM_ERASEBKGND:
		  return TRUE;
	}

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

HWND html_create (HWND dad, const char *format, ...)
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



BOOL CALLBACK findelembyid (HELEMENT he, LPVOID param)
{
	*(HELEMENT *)param = he;
	return TRUE;
}

static HELEMENT html_getelementbyid (HWND hwnd, const char *id)
{
	HELEMENT hroot, he;
	char selector[128];
	int e;

	e = HTMLayoutGetRootElement (hwnd, &hroot);
	if (e != HLDOM_OK) return NULL;

	sprintf (selector, "*[id=%s]", id);
	e = HTMLayoutSelectElements (hroot, selector, findelembyid, &he);
	if (e != HLDOM_OK) return NULL;

	return he;
}




static char *dump(char *s)
{
	char *t;
	int n = 0;

	if (s) n = strlen(s);
	t = (char *)malloc(n+1);
	if (s) strcpy(t, s);
	t[n] = '\0';
	return t;
}

static wchar_t *utf8tou (char *utf8)
{
	int len;
	wchar_t *wbuf;
	int wlen;

	len = strlen(utf8);
	wbuf = (wchar_t *)malloc((len+1)*sizeof(wchar_t));
	wlen = MultiByteToWideChar (CP_UTF8, 0, utf8, len, wbuf, len*sizeof(wchar_t) );
	wbuf[wlen] = L'\0';

	return wbuf;
}

static void wfree (wchar_t *ws)
{
	if (ws) free(ws);
}

static char *uto8 (wchar_t *ws)
{
	char *s;
	int size;

	size = WideCharToMultiByte(CP_UTF8, 0, ws, -1, NULL, 0, NULL, FALSE);
	s = (char *)malloc(size);
	WideCharToMultiByte (CP_UTF8, 0, ws, -1, s, size, NULL, FALSE);
	s[size] = '\0';
	return s;
}

static void mfree (char *s)
{
	if (s) free(s);
}


static char *trim (char *s)
{
	int n;
	char *start, *end;

	if (!s) return NULL;

	for (start = s; *start && isspace(*start); start++);

	for (end = s+strlen(s)-1; end>start && isspace(*end); end--);

	n = end - start;
	if (n < 0)
		start = (char *)"";
	else
		start[n+1] = '\0';

	start = dump(start);
	free(s);

	return start;
}




/* 获得元素在父元素的第几个位置 */
int html_getnth (HELEMENT he)
{
	int e;
	UINT i, n;
	HELEMENT hp;

	e = HTMLayoutGetParentElement(he, &hp);
	if (e != HLDOM_OK) return 0;

	HTMLayoutGetChildrenCount(hp, &n);
	for (i = 0; i <= n; i++) {
		HELEMENT htmp;
		HTMLayoutGetNthChild(hp, i, &htmp);
		if (htmp == he)
			return i;
	}

	return 0;
}

/* 展开/收起item事件 */
BOOL CALLBACK itemproc (LPVOID tag, HELEMENT he, UINT evtg, LPVOID prms )
{
	static HELEMENT expandelem = NULL; /* 已展开的item */

	struct MOUSE_PARAMS *ep = (struct MOUSE_PARAMS *)prms;
	HWND hwnd = (HWND)tag;
	int e;

	if ((evtg & HANDLE_MOUSE) && ep->cmd==MOUSE_DOWN && ep->button_state==MAIN_MOUSE_BUTTON) {

		html_hidetip (hwnd);

		HELEMENT hitemcon = html_getelementbyid(hwnd, "itemcontent");
		HELEMENT hitemlist = html_getelementbyid(hwnd, "itemlist");

		/* 收起 */
		if (expandelem == he) {
			HTMLayoutSetStyleAttribute(hitemcon, "display", L"none");
			HTMLayoutUpdateElement(hitemlist, TRUE);
			expandelem = NULL;
			return TRUE;
		}

		/* 展开 */
		int itemid;
		wchar_t *wid;
		e = HTMLayoutGetAttributeByName(he, "itemid", (const WCHAR **)&wid);
		if (e != HLDOM_OK) return TRUE;
		swscanf (wid, L"%d", &itemid);
		if (itemid <= 0) return TRUE;

		FeedItem *item = db_loaditem(itemid);
		char *html = item->content;
		HTMLayoutSetElementHtml(hitemcon, (const BYTE *)html, strlen(html), SIH_REPLACE_CONTENT);
		delitem(item);

		/* 标记为已读 */
		HTMLayoutSetStyleAttribute (he, "font-weight", L"normal");
		db_markread (itemid, 1);
		html_refresh_feedlist (hwnd); /* 刷新左侧 */

		HTMLayoutSetStyleAttribute(hitemcon, "display", L"block");
		int n = html_getnth (he);
		HTMLayoutInsertElement(hitemcon, hitemlist, n+1);
		HTMLayoutScrollToView(he, SCROLL_SMOOTH);
		HTMLayoutUpdateElement(hitemlist, TRUE);

		expandelem = he;

		return TRUE;
	}

	return FALSE;
}

/* 插入单条item */
void html_insertitem (HWND hwnd, FeedItem *item)
{
	HELEMENT hitemlist, hdt;
	uint n;

	if (!item) return;

	hitemlist = html_getelementbyid (hwnd, "itemlist");
	if (!hitemlist) return;

	/* create item element */ {
		wchar_t *wtitle = utf8tou(item->title);
		HTMLayoutCreateElement ("dt", wtitle, &hdt); /* <dt itemid=''> */
		wfree(wtitle);
		wchar_t wid[10];
		wsprintf (wid, L"%d", item->id);
		HTMLayoutSetAttributeByName (hdt, "itemid", wid);
	}
	HTMLayoutGetChildrenCount (hitemlist, &n);
	HTMLayoutInsertElement (hdt, hitemlist, n+1);
	if (item->read == 0)
		HTMLayoutSetStyleAttribute(hdt, "font-weight", L"bold");

	/* 挂展开事件 */
	HTMLayoutAttachEventHandler (hdt, itemproc, (LPVOID)hwnd);
}

/* 载入item列表渲染到html, by feedid, 0为所有 */
void html_loaditemlist (HWND hwnd, int feedid)
{
	FeedItem **itemlist;
	Feed *feed;
	int rown;

	if (feedid != 0) {
		feed = db_loadfeed (feedid);
		if (!feed) return;
		printf ("fid: %s\n", feed->link);
		html_setboard (hwnd, feed->title);
		delfeed(feed);
	}
	else
		html_setboard (hwnd, "所有");

	if (feedid == 0)
		rown = db_select_itemlist(&itemlist, "read=0 order by updated desc, id desc");
	else
		rown = db_select_itemlist(&itemlist, "read=0 and feedid=%d order by updated desc, id desc", feedid);

	for (int i = 0; i < rown; i++) {
		FeedItem *item = itemlist[i];
		html_insertitem (hwnd, item);
	}

	db_del_itemlist(itemlist, rown);
}

/* 清除所有item */
void html_clearitemlist (HWND hwnd)
{
	HELEMENT hitemlist, hcon, hdt;
	UINT i, n;

	hitemlist = html_getelementbyid (hwnd, "itemlist");
	if (!hitemlist) return;

	hcon = html_getelementbyid (hwnd, "itemcontent");
	if (!hcon) return;

	HTMLayoutSetStyleAttribute (hcon, "display", L"none");
	HTMLayoutInsertElement(hcon, hitemlist, 0);

	HTMLayoutGetChildrenCount(hitemlist, &n);
	for (i = 1; i < n; i++) {
		HTMLayoutGetNthChild(hitemlist, 1, &hdt);
		HTMLayoutDetachEventHandler(hdt, itemproc, (LPVOID)hwnd);
		HTMLayoutDeleteElement(hdt);
	}
}



/* 点击feedlist事件 */
BOOL CALLBACK feedproc (LPVOID tag, HELEMENT he, UINT evtg, LPVOID prms)
{
	HWND hwnd = (HWND)tag;
	struct MOUSE_PARAMS *ep = (struct MOUSE_PARAMS *)prms;
	int e;

	if ((evtg&HANDLE_MOUSE) && ep->cmd==MOUSE_DOWN && ep->button_state==MAIN_MOUSE_BUTTON) {

		html_hidetip (hwnd);

		int feedid;
		wchar_t *wid;
		e = HTMLayoutGetAttributeByName(he, "feedid", (const WCHAR **)&wid);
		if (e != HLDOM_OK) return FALSE;
		swscanf (wid, L"%d", &feedid);

		html_clearitemlist(hwnd);
		html_loaditemlist(hwnd, feedid);

		return FALSE;
	}

	return FALSE;
}

/* 插入单条feed */
void html_insertfeed (HWND hwnd, Feed *feed)
{
	HELEMENT hroot, hfeedlist, hdt;
	int e;
	uint n, unreadn;

	if (!feed) return;

	e = HTMLayoutGetRootElement(hwnd, &hroot);
	if (e != HLDOM_OK) return;

	hfeedlist = html_getelementbyid (hwnd, "feedlist");
	if (!hfeedlist) return;

	/* utf8 to unicode */ {
		unreadn = db_unreadcount(feed->id);
		char buf[128], *title = feed->title;
		if (unreadn > 0) {
			sprintf (buf, "%s(%d)", title, unreadn);
			title = buf;
		}
		wchar_t *wtitle = utf8tou(title);
		HTMLayoutCreateElement("option", wtitle, &hdt);
		wfree (wtitle);
		wchar_t wid[10];
		wsprintf (wid, L"%d", feed->id);
		HTMLayoutSetAttributeByName(hdt, "feedid", wid);
	}

	HTMLayoutGetChildrenCount (hfeedlist, &n);
	HTMLayoutInsertElement(hdt, hfeedlist, n+1);
	if (unreadn > 0)
		HTMLayoutSetStyleAttribute (hdt, "font-weight", L"bold");

	HTMLayoutAttachEventHandler(hdt, feedproc, (LPVOID)hwnd);
}

/* 载入所有feed到左侧 */
void html_loadfeedlist (HWND hwnd)
{
	Feed **feedlist;
	Feed feed;
	int rown;

	feed.id = 0;
	feed.title = (char *)"全部";
	html_insertfeed (hwnd, &feed);

	rown = db_select_feedlist (&feedlist, "1");
	for (int i = 0; i < rown; i++) {
		Feed *feed = feedlist[i];
		html_insertfeed (hwnd, feed);
	}

	db_del_feedlist (feedlist, rown);
}

/* 清除所有item */
void html_clearfeedlist (HWND hwnd)
{
	HELEMENT hfeedlist, hdt;
	UINT i, n;

	hfeedlist = html_getelementbyid (hwnd, "feedlist");
	if (!hfeedlist) return;

	HTMLayoutGetChildrenCount(hfeedlist, &n);
	for (i = 0; i < n; i++) {
		HTMLayoutGetNthChild(hfeedlist, 0, &hdt);
		HTMLayoutDetachEventHandler(hdt, feedproc, (LPVOID)hwnd);
		HTMLayoutDeleteElement(hdt);
	}
}

void html_refresh_feedlist (HWND hwnd)
{
	html_clearfeedlist (hwnd);
	html_loadfeedlist (hwnd);
}

/* 设置右侧标题 */
void html_setboard (HWND hwnd, const char *feedtitle)
{
	HELEMENT hboard;

	if (!feedtitle) return;

	hboard = html_getelementbyid(hwnd, "board");
	if (!hboard) return;

	HTMLayoutSetElementHtml(hboard, (const BYTE *)feedtitle, strlen(feedtitle), SIH_REPLACE_CONTENT);
}

/**
 * base functions
 */
int html_loadfile (HWND hwnd, const wchar_t *filename)
{
	wchar_t path[2048];
	int len, i;

	// 获得绝对路径
	len = GetModuleFileNameW (NULL, path, sizeof(path)/sizeof(wchar_t));
	if (len <= 0) return 0;
	for (i = len; i >= 0 && (path[i]!=L'\\' && path[i]!=L'/'); i--);
	path[i+1] = L'\0';
	lstrcat (path, filename);

	//MessageBox (NULL, path, path, MB_OK);
	HTMLayoutSetOption (hwnd, HTMLAYOUT_FONT_SMOOTHING, 2);
	HTMLayoutSetOption (hwnd, HTMLAYOUT_SMOOTH_SCROLL, FALSE);
	return HTMLayoutLoadFile(hwnd, path);
}

int html_loadstring (HWND hwnd, char *html)
{
	return HTMLayoutLoadHtml(hwnd, (LPCBYTE)html, strlen(html));
}



/* 订阅按钮 */
static html_subcb g_clicksub = NULL;

static BOOL CALLBACK click_showmenu (LPVOID tag, HELEMENT he, UINT evtg, LPVOID prms)
{
	HWND hwnd = (HWND)tag;
	struct MOUSE_PARAMS *ep = (struct MOUSE_PARAMS *)prms;

	if ((evtg&HANDLE_MOUSE) && (BYTE)ep->cmd==MOUSE_UP && ep->button_state==MAIN_MOUSE_BUTTON) {
		HELEMENT hmenu = html_getelementbyid(hwnd, "subpad");
		if (!hmenu) return FALSE;
		HTMLayoutSetStyleAttribute (hmenu, "display", L"block");
		return FALSE;
	}
	return FALSE;
}

static BOOL CALLBACK click_closemenu (LPVOID tag, HELEMENT he, UINT evtg, LPVOID prms)
{
	HWND hwnd = (HWND)tag;
	struct MOUSE_PARAMS *ep = (struct MOUSE_PARAMS *)prms;

	if ((evtg&HANDLE_MOUSE) && (BYTE)ep->cmd==MOUSE_UP && ep->button_state==MAIN_MOUSE_BUTTON) {
		HELEMENT hmenu = html_getelementbyid(hwnd, "subpad");
		if (!hmenu) return FALSE;
		HTMLayoutSetStyleAttribute (hmenu, "display", L"none");
		return FALSE;
	}
	return FALSE;
}

/* 确定 */
static BOOL CALLBACK click_sub (LPVOID tag, HELEMENT he, UINT evtg, LPVOID prms)
{
	HWND hwnd = (HWND)tag;
	struct MOUSE_PARAMS *ep = (struct MOUSE_PARAMS *)prms;

	if ((evtg&HANDLE_MOUSE) && (BYTE)ep->cmd==MOUSE_UP && ep->button_state==MAIN_MOUSE_BUTTON) {

		HELEMENT hmenu = html_getelementbyid(hwnd, "subpad");
		if (!hmenu) return FALSE;
		/* 避免重复事件 */
		wchar_t *dispmenu;
		HTMLayoutGetStyleAttribute(hmenu, "display", (const WCHAR **)&dispmenu);
		if (lstrcmp(dispmenu, L"none") == 0)
			return FALSE;
		HTMLayoutSetStyleAttribute (hmenu, "display", (WCHAR *)L"none");

		wchar_t url[2048];
		UINT urllen = sizeof(url)/sizeof(wchar_t);
		HELEMENT hinput = html_getelementbyid(hwnd, "inputurl");
		HTMLayoutGetElementText(hinput, url, &urllen);
		if (urllen > 0) {
			url[urllen] = L'\0';
			char *s = uto8(url);
			s = trim(s);
			if (g_clicksub) g_clicksub(hwnd, s);
			mfree(s);
		}
		return FALSE;
	}
	return FALSE;
}

void html_set_subevt (HWND hwnd, html_subcb f)
{
	HELEMENT submenu = html_getelementbyid (hwnd, "submenu");
	if (!submenu) return;
	HTMLayoutAttachEventHandler(submenu, click_showmenu, (LPVOID)hwnd);

	HELEMENT closemenu = html_getelementbyid (hwnd, "closemenu");
	if (!closemenu) return;
	HTMLayoutAttachEventHandler(closemenu, click_closemenu, (LPVOID)hwnd);

	HELEMENT addbutton = html_getelementbyid (hwnd, "addbutton");
	if (!addbutton) return;
	g_clicksub = f;
	HTMLayoutAttachEventHandler(addbutton, click_sub, (LPVOID)hwnd);

}



/* 加载提示框, state=1:黄, 0:红 */
void html_showtip(HWND hwnd, const char *info, int state)
{
	HELEMENT tipbox = html_getelementbyid(hwnd, "loadtip");
	if (!tipbox) return;

	const wchar_t *color = state? L"#FFEA93" : L"#FECFDC";
	HTMLayoutSetStyleAttribute(tipbox, "border-color", color);
	HTMLayoutSetStyleAttribute(tipbox, "background-color", color);
	HTMLayoutSetElementHtml(tipbox, (const BYTE *)info, strlen(info), SIH_REPLACE_CONTENT);
	HTMLayoutSetStyleAttribute(tipbox, "display", L"block");
	//HTMLayoutUpdateElement(tipbox, TRUE);
	HTMLayoutUpdateWindow(hwnd);
}

void html_hidetip(HWND hwnd)
{
	HELEMENT tipbox = html_getelementbyid(hwnd, "loadtip");
	if (!tipbox) return;

	HTMLayoutSetStyleAttribute(tipbox, "display", L"none");
}


