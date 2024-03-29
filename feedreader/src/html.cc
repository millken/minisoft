/*
 * 附加的html浏览控件
 */

#include <ctype.h>
#include "i32.h"
#include <commdlg.h>
#include "htmlayout/htmlayout.h"
#include "htmlayout/behaviors/notifications.h" // hyperlink behavior notfication is here

#include "html.h"
#include "xml.h"
#include "db.h"
#include "url.h"

static int g_current_feedid = 0; /* 正在阅读的feed */
static int g_menu_feedid = 0; /* 左侧菜单对应的feedid */
static BOOL g_onleft = 0; /* 左侧菜单是否出现 */

static HELEMENT g_lastexpand = NULL; /* 用于标记展开收起 */

/* 订阅按钮 */
static html_subcb g_clicksub = NULL;
static html_logincb g_logincb = NULL;


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

static wchar_t *utf8tou (const char *utf8)
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
	struct MOUSE_PARAMS *ep = (struct MOUSE_PARAMS *)prms;
	HWND hwnd = (HWND)tag;
	int e;

	if ((evtg & HANDLE_MOUSE) && ep->cmd==MOUSE_DOWN && ep->button_state==MAIN_MOUSE_BUTTON) {

		html_hidetip (hwnd);

		HELEMENT hitemcon = html_getelementbyid(hwnd, "itemcontent");
		HELEMENT ctitle = html_getelementbyid(hwnd, "ctitle");
		HELEMENT cnote = html_getelementbyid(hwnd, "cnote");
		HELEMENT ccon = html_getelementbyid(hwnd, "ccon");
		HELEMENT hitemlist = html_getelementbyid(hwnd, "itemlist");

		/* 收起 */
		if (g_lastexpand == he) {
			HTMLayoutSetStyleAttribute(hitemcon, "display", L"none");
			HTMLayoutUpdateElement(hitemlist, TRUE);
			g_lastexpand = NULL;
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
		if (!item) return TRUE;
		char *html = item->content;
		char notebuf[128], *note = item->author;
		if (strlen(item->author) > 0) {
			sprintf (notebuf, "作者: %s", note);
			note = notebuf;
		}
		HTMLayoutSetElementInnerText(ctitle, (const BYTE *)item->title, strlen(item->title));
		wchar_t *link = utf8tou(item->link);
		HTMLayoutSetAttributeByName(ctitle, "href", (const WCHAR *)link);
		wfree(link);
		HTMLayoutSetElementInnerText(cnote, (const BYTE *)note, strlen(note));

		//updated time
		HELEMENT hctime = html_getelementbyid(hwnd, "ctime");
		if (hctime) {
			char timebuf[64];
			struct tm* t = localtime(&item->updated);
			sprintf (timebuf, "%d-%d-%d %d:%02d:%02d", t->tm_year+1900, t->tm_mon+1, t->tm_mday,
					t->tm_hour, t->tm_min, t->tm_sec);
			HTMLayoutSetElementInnerText(hctime, (const BYTE*)timebuf, strlen(timebuf));
		}

		HTMLayoutSetElementHtml(ccon, (const BYTE *)item->content, strlen(item->content), SIH_REPLACE_CONTENT);
		//HTMLayoutSetElementHtml(hitemcon, (const BYTE *)html, strlen(html), SIH_REPLACE_CONTENT);
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

		g_lastexpand = he;

		return TRUE;
	}

	/* 右键切换已读未读状态 */
	if ((evtg & HANDLE_MOUSE) && ep->cmd==MOUSE_UP && ep->button_state==PROP_MOUSE_BUTTON) {

		int itemid;
		wchar_t *wid;
		e = HTMLayoutGetAttributeByName(he, "itemid", (const WCHAR **)&wid);
		if (e != HLDOM_OK) return TRUE;
		swscanf (wid, L"%d", &itemid);
		if (itemid <= 0) return TRUE;

		FeedItem *item = db_loaditem(itemid);
		if (!item) return TRUE;

		db_markread (item->id, !item->read);

		HELEMENT itemcon = html_getelementbyid (hwnd, "itemcontent");
		if (itemcon)
			HTMLayoutSetStyleAttribute (itemcon, "display", (const WCHAR *)L"none");

		html_refresh_feedlist(hwnd);
		if (item->read)
			HTMLayoutSetStyleAttribute(he, "font-weight", (const WCHAR *)L"bold");
		else
			HTMLayoutSetStyleAttribute(he, "font-weight", (const WCHAR *)L"normal");

		HTMLayoutUpdateElement(he, TRUE);
	}

	return FALSE;
}

/* 插入单条item */
void html_insertitem (HWND hwnd, FeedItem *item)
{
	HELEMENT hitemlist, hdt, span, i;
	uint n;

	if (!item) return;

	hitemlist = html_getelementbyid (hwnd, "itemlist");
	if (!hitemlist) return;

	/* create item element */ {
	HTMLayoutCreateElement ("dt", NULL, &hdt); /* <dt itemid=''> */
	wchar_t wid[10];
	wsprintf (wid, L"%d", item->id);
	HTMLayoutSetAttributeByName (hdt, "itemid", wid);
	}

	{
	wchar_t *wtitle = utf8tou(item->title);
	HTMLayoutCreateElement ("span", wtitle, &span);
	wfree(wtitle);
	HTMLayoutInsertElement (span, hdt, 0);
	}

	{
	struct tm* t = localtime(&item->updated);
	char timebuf[128];
	sprintf (timebuf, "%d-%d-%d", t->tm_year+1900, t->tm_mon+1, t->tm_mday);
	//printf ("%s\n", timebuf);
	wchar_t *wtime = utf8tou(timebuf);
	HTMLayoutCreateElement ("i", wtime, &i);
	wfree(wtime);
	HTMLayoutInsertElement (i, hdt, 1);
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

	g_current_feedid = feedid;

	if (feedid != 0) {
		feed = db_loadfeed (feedid);
		if (!feed) return;
		printf ("fid: %s\n", feed->link);
		html_setboard (hwnd, feed->title, feed->link);
		delfeed(feed);
	}
	else
		html_setboard (hwnd, "所有", "");

	if (feedid == 0)
		rown = db_select_itemlist(&itemlist, "1 order by read, updated desc, id desc limit 400");
	else
		rown = db_select_itemlist(&itemlist, "feedid=%d order by read,updated desc, id desc limit 200", feedid);

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


static
BOOL CALLBACK enum_item ( LPVOID p, HELEMENT he, int pos, int postype, WCHAR code )
{
	const char *tag;

	HTMLayoutGetElementType(he, &tag);
	if (strcmp(tag, "dt") == 0) {
		*(HELEMENT *)p = he;
		return TRUE;
	}
	return FALSE;
}

/* 获得第一个item */
static
HELEMENT get_first_item (HWND hwnd)
{
	HELEMENT hitemlist = html_getelementbyid(hwnd, "itemlist");
	if (!hitemlist) return NULL;

	HELEMENT hitem = NULL;
	HTMLayoutEnumerate(hitemlist, enum_item, &hitem, FALSE);
	return hitem;
}



/* 点击feedlist事件 */
BOOL CALLBACK feedproc (LPVOID tag, HELEMENT he, UINT evtg, LPVOID prms)
{
	HWND hwnd = (HWND)tag;
	struct MOUSE_PARAMS *ep = (struct MOUSE_PARAMS *)prms;
	int e;

	/* 左键 */
	if ((evtg&HANDLE_MOUSE) && ep->cmd==MOUSE_DOWN && ep->button_state==MAIN_MOUSE_BUTTON) {

		html_hidetip (hwnd);

		int feedid;
		wchar_t *wid;
		e = HTMLayoutGetAttributeByName(he, "feedid", (const WCHAR **)&wid);
		if (e != HLDOM_OK) return FALSE;
		swscanf (wid, L"%d", &feedid);

		html_clearitemlist(hwnd);
		html_loaditemlist(hwnd, feedid);

		HELEMENT hitemlist = html_getelementbyid(hwnd, "itemlist");
		HELEMENT dt;
		dt = get_first_item (hwnd);
		//HTMLayoutGetNthChild(hitemlist, 1, &dt);
		HTMLayoutScrollToView(dt, SCROLL_TO_TOP);
		HTMLayoutUpdateElement(hitemlist, TRUE);
		return FALSE;
	}

	/* 右键 */
	if ((evtg&HANDLE_MOUSE) && ep->cmd==MOUSE_DOWN && ep->button_state==PROP_MOUSE_BUTTON) {

		int feedid;
		wchar_t *wid;
		e = HTMLayoutGetAttributeByName(he, "feedid", (const WCHAR **)&wid);
		if (e != HLDOM_OK) return FALSE;
		swscanf (wid, L"%d", &feedid);
		g_menu_feedid = feedid;

		HELEMENT unsub = html_getelementbyid (hwnd, "unsub");
		if (unsub) {
			if (feedid == 0)
				HTMLayoutSetStyleAttribute(unsub, "display", (const WCHAR *)L"none");
			else
				HTMLayoutSetStyleAttribute(unsub, "display", (const WCHAR *)L"block");
		}

		g_onleft = TRUE;
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
		int e = HTMLayoutGetNthChild(hfeedlist, 0, &hdt);
		if (e != HLDOM_OK) continue;
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
void html_setboard (HWND hwnd, const char *feedtitle, const char *feedlink)
{
	HELEMENT hboard;

	if (!feedtitle) return;

	hboard = html_getelementbyid(hwnd, "boardtitle");
	if (!hboard) return;

	HTMLayoutSetElementInnerText(hboard, (const BYTE *)feedtitle, strlen(feedtitle));
	wchar_t *wlink = utf8tou(feedlink);
	HTMLayoutSetAttributeByName(hboard, "href", wlink);
	wfree(wlink);
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

/* 处理超连接 */
static BOOL CALLBACK click_linker (LPVOID tag, HELEMENT he, UINT evtg, LPVOID prms)
{
	static DWORD lasttime = 0;
	DWORD now;

	HWND hwnd = (HWND)tag;
	struct BEHAVIOR_EVENT_PARAMS *ep = (struct BEHAVIOR_EVENT_PARAMS *)prms;


	if ((evtg&HANDLE_BEHAVIOR_EVENT) && (BYTE)ep->cmd==HYPERLINK_CLICK) {
		now = GetTickCount();
		if (now - lasttime < 100)
			return TRUE;

		wchar_t *link;
		HTMLayoutGetAttributeByName(ep->he, "href", (const WCHAR **)&link);
		if (lstrlen(link) > 4)
			ShellExecuteW (0, L"open", link, NULL, NULL, SW_NORMAL);
		lasttime = now;
		return TRUE;
	}
	return FALSE;
}

/* 取消订阅 */
static BOOL CALLBACK click_unsub (LPVOID tag, HELEMENT he, UINT evtg, LPVOID prms)
{
	static DWORD lasttime = 0;

	HWND hwnd = (HWND)tag;
	struct MOUSE_PARAMS *ep = (struct MOUSE_PARAMS *)prms;

	if ((evtg&HANDLE_MOUSE) && (BYTE)ep->cmd==MOUSE_UP && ep->button_state==MAIN_MOUSE_BUTTON) {
		DWORD now = GetTickCount();
		if (now - lasttime < 100)
			return FALSE;
		else
			lasttime = now;

		db_unsub(g_menu_feedid);
		html_clearfeedlist (hwnd);
		html_loadfeedlist (hwnd);
		if (g_menu_feedid==g_current_feedid || g_current_feedid==0) {
			html_clearitemlist (hwnd);
			html_loaditemlist (hwnd, 0);
		}

		g_onleft = FALSE;
		return FALSE;
	}

	return FALSE;
}

/* 把所有标记为已读 */
static BOOL CALLBACK click_markread (LPVOID tag, HELEMENT he, UINT evtg, LPVOID prms)
{
	static DWORD lasttime = 0;

	HWND hwnd = (HWND)tag;
	struct MOUSE_PARAMS *ep = (struct MOUSE_PARAMS *)prms;

	if ((evtg&HANDLE_MOUSE) && (BYTE)ep->cmd==MOUSE_UP && ep->button_state==MAIN_MOUSE_BUTTON) {
		DWORD now = GetTickCount();
		if (now - lasttime < 100)
			return FALSE;
		else
			lasttime = now;

		db_markallread(g_menu_feedid);
		html_clearfeedlist (hwnd);
		html_loadfeedlist (hwnd);
		html_clearitemlist (hwnd);
		html_loaditemlist (hwnd, g_menu_feedid);

		g_onleft = FALSE;
		return FALSE;
	}

	return FALSE;
}

/* 鼠标在左侧 */
static BOOL CALLBACK on_feedlist (LPVOID tag, HELEMENT he, UINT evtg, LPVOID prms)
{
	static DWORD lasttime = 0;

	HWND hwnd = (HWND)tag;
	struct MOUSE_PARAMS *ep = (struct MOUSE_PARAMS *)prms;

	if ((evtg&HANDLE_MOUSE) && (BYTE)ep->cmd==MOUSE_ENTER) {
		g_onleft = TRUE;
		return FALSE;
	}

	if ((evtg&HANDLE_MOUSE) && (BYTE)ep->cmd==MOUSE_LEAVE) {
		g_onleft = FALSE;
		return FALSE;
	}

	return FALSE;
}

/* 打开登录框 */
static BOOL CALLBACK click_loginlink (LPVOID tag, HELEMENT he, UINT evtg, LPVOID prms)
{
	HWND hwnd = (HWND)tag;
	struct MOUSE_PARAMS *ep = (struct MOUSE_PARAMS *)prms;

	if ((evtg&HANDLE_MOUSE) && (BYTE)ep->cmd==MOUSE_UP && ep->button_state==MAIN_MOUSE_BUTTON) {

		HELEMENT loginform = html_getelementbyid(hwnd, "loginform");
		if (loginform) {
			HTMLayoutSetStyleAttribute(loginform, "display", L"block");
			printf ("s");
		}
		HELEMENT usernamebox = html_getelementbyid(hwnd, "username");
		HTMLayoutSetElementState(usernamebox, STATE_FOCUS, 0, TRUE);
		return FALSE;
	}

	return FALSE;
}

/* 关闭登录框 */
static BOOL CALLBACK click_closeform (LPVOID tag, HELEMENT he, UINT evtg, LPVOID prms)
{
	HWND hwnd = (HWND)tag;
	struct MOUSE_PARAMS *ep = (struct MOUSE_PARAMS *)prms;

	if ((evtg&HANDLE_MOUSE) && (BYTE)ep->cmd==MOUSE_UP && ep->button_state==MAIN_MOUSE_BUTTON) {

		HELEMENT loginform = html_getelementbyid(hwnd, "loginform");
		if (loginform)
			HTMLayoutSetStyleAttribute(loginform, "display", L"none");
		html_hidetip (hwnd);
		return FALSE;
	}

	return FALSE;
}

/* right board on mouse hover */
static BOOL CALLBACK hover_board (LPVOID tag, HELEMENT he, UINT evtg, LPVOID prms)
{
	HWND hwnd = (HWND)tag;
	struct MOUSE_PARAMS *ep = (struct MOUSE_PARAMS *)prms;

	if ((evtg&HANDLE_MOUSE) && (BYTE)ep->cmd==MOUSE_ENTER) {
		HELEMENT markbtn = html_getelementbyid(hwnd, "markbtn");
		HTMLayoutSetStyleAttribute(markbtn, "display", L"block");
	}
	else if ((evtg&HANDLE_MOUSE) && (BYTE)ep->cmd==MOUSE_LEAVE) {
		HELEMENT markbtn = html_getelementbyid(hwnd, "markbtn");
		HTMLayoutSetStyleAttribute(markbtn, "display", L"none");
	}

	return FALSE;
}

void html_loginsucess (HWND hwnd, const char *username)
{
	HELEMENT hlogen = html_getelementbyid (hwnd, "loginuser");
	HELEMENT hloglink = html_getelementbyid (hwnd, "loginlink");
	HELEMENT hlogform = html_getelementbyid (hwnd, "loginform");
	if (hlogen) {
		HTMLayoutSetElementInnerText(hlogen, (const BYTE *)username, strlen(username));
		HTMLayoutSetStyleAttribute(hlogen, "display", L"block");
	}
	if (hloglink)
		HTMLayoutSetStyleAttribute(hloglink, "display", L"none");
	if (hlogform)
		HTMLayoutSetStyleAttribute(hlogform, "display", L"none");
}

static BOOL login (HWND hwnd)
{
		char *username, *password;
		HELEMENT husername = html_getelementbyid(hwnd, "username");
		HTMLayoutGetElementInnerText(husername, (BYTE **)&username);
		HELEMENT hpwd = html_getelementbyid(hwnd, "password");
		HTMLayoutGetElementInnerText(hpwd, (BYTE **)&password);

		/* 表单验证 */
		html_hidetip (hwnd);
		if (strlen(username)==0 ) {
			html_showtip(hwnd, "请填写用户名", 0);
			return TRUE;
		}
		if (strlen(password)==0 ) {
			html_showtip(hwnd, "密码不能为空", 0);
			return TRUE;
		}

		HELEMENT loginform = html_getelementbyid(hwnd, "loginform");
		HTMLayoutSetStyleAttribute(loginform, "display", L"none");
		html_showtip (hwnd, "正在登录..", 1);

		struct user *u = url_login (username, password);
		if (!u) {
			url_deluser (u);
			html_showtip(hwnd, "登录失败 :(", 0);
			HTMLayoutSetStyleAttribute(loginform, "display", L"block");
			return TRUE;
		}

		int e = 0;
		if (g_logincb)
			e = g_logincb (hwnd, u->suid, u->username);

		if (!e)
			html_showtip (hwnd, "噢,发生错误!", 0);
		else {
			html_showtip(hwnd, "登录成功!", 1);
			Sleep(700);
			html_hidetip(hwnd);
		}

		url_deluser (u);
		return FALSE;
}

/* 点登录按钮 */
static BOOL CALLBACK click_loginbutton (LPVOID tag, HELEMENT he, UINT evtg, LPVOID prms)
{
	static DWORD lasttime = 0;

	HWND hwnd = (HWND)tag;
	//struct MOUSE_PARAMS *ep = (struct MOUSE_PARAMS *)prms;
	struct BEHAVIOR_EVENT_PARAMS *ep = (struct BEHAVIOR_EVENT_PARAMS *)prms;

	//if ((evtg&HANDLE_MOUSE) && (BYTE)ep->cmd==MOUSE_DOWN && ep->button_state==MAIN_MOUSE_BUTTON) {
	if ((evtg&HANDLE_BEHAVIOR_EVENT) && (BYTE)ep->cmd==BUTTON_PRESS) {
		DWORD now = GetTickCount();
		if (now - lasttime < 500)
			return TRUE;

		login (hwnd);

		lasttime = now = GetTickCount();
		return TRUE;
	}
	return FALSE;
}

/* 点全部读过按钮 */
static BOOL CALLBACK click_markbtn (LPVOID tag, HELEMENT he, UINT evtg, LPVOID prms)
{
	static DWORD lasttime = 0;

	HWND hwnd = (HWND)tag;
	//struct MOUSE_PARAMS *ep = (struct MOUSE_PARAMS *)prms;
	struct BEHAVIOR_EVENT_PARAMS *ep = (struct BEHAVIOR_EVENT_PARAMS *)prms;

	//if ((evtg&HANDLE_MOUSE) && (BYTE)ep->cmd==MOUSE_DOWN && ep->button_state==MAIN_MOUSE_BUTTON) {
	if ((evtg&HANDLE_BEHAVIOR_EVENT) && (BYTE)ep->cmd==BUTTON_PRESS) {
		DWORD now = GetTickCount();
		if (now - lasttime < 500)
			return TRUE;

		db_markallread (g_current_feedid);
		html_refresh_feedlist(hwnd);
		html_clearitemlist(hwnd);
		html_loaditemlist(hwnd, g_current_feedid);

		lasttime = now = GetTickCount();
		return TRUE;
	}
	return FALSE;
}


/* 设置订阅按钮回调 */
void html_set_subevt (HWND hwnd, html_subcb f)
{
	HELEMENT addbutton = html_getelementbyid (hwnd, "addbutton");
	if (addbutton) {
		g_clicksub = f;
		HTMLayoutAttachEventHandler(addbutton, click_sub, (LPVOID)hwnd);
	}

}

/* 设置登录后回调 */
void html_set_logincb (HWND hwnd, html_logincb f)
{
	HELEMENT loginbutton = html_getelementbyid (hwnd, "loginbutton");
	if (loginbutton) {
		g_logincb = f;
		HTMLayoutAttachEventHandler(loginbutton, click_loginbutton, (LPVOID)hwnd);
	}

}

/* 回车提交 */
static BOOL CALLBACK keydown_password (LPVOID tag, HELEMENT he, UINT evtg, LPVOID prms)
{
	static DWORD lasttime = 0;

	HWND hwnd = (HWND)tag;
	struct KEY_PARAMS *ep = (struct KEY_PARAMS *)prms;

	if ((evtg&HANDLE_KEY) && (BYTE)ep->cmd==KEY_DOWN && ep->key_code==VK_RETURN) {
		DWORD now = GetTickCount();
		if (now - lasttime < 100) return FALSE;
		login (hwnd);
		lasttime = now = GetTickCount();
		return FALSE;
	}

	return FALSE;
}

/* 折叠 */
static BOOL CALLBACK click_fold (LPVOID tag, HELEMENT he, UINT evtg, LPVOID prms)
{
	static DWORD lasttime = 0;

	HWND hwnd = (HWND)tag;
	struct MOUSE_PARAMS *ep = (struct MOUSE_PARAMS *)prms;

	if ((evtg&HANDLE_MOUSE) && (BYTE)ep->cmd==MOUSE_UP && ep->button_state==MAIN_MOUSE_BUTTON) {
		DWORD now = GetTickCount();
		if (now - lasttime < 100)
			return FALSE;
		else
			lasttime = now;

		HELEMENT hcont = html_getelementbyid(hwnd, "itemcontent");
		if (hcont)
			HTMLayoutSetStyleAttribute(hcont, "display", L"none");

		g_lastexpand = NULL;
		return FALSE;
	}

	return FALSE;
}

void html_init (HWND hwnd)
{
	HELEMENT submenu = html_getelementbyid (hwnd, "submenu");
	if (submenu)
		HTMLayoutAttachEventHandler(submenu, click_showmenu, (LPVOID)hwnd);

	HELEMENT closemenu = html_getelementbyid (hwnd, "closemenu");
	if (closemenu)
		HTMLayoutAttachEventHandler(closemenu, click_closemenu, (LPVOID)hwnd);

	HELEMENT body = html_getelementbyid (hwnd, "all");
	if (body)
		HTMLayoutAttachEventHandler(body, click_linker, (LPVOID)hwnd);

	HELEMENT unsub = html_getelementbyid (hwnd, "unsub");
	if (unsub)
		HTMLayoutAttachEventHandler(unsub, click_unsub, (LPVOID)hwnd);

	HELEMENT markread = html_getelementbyid (hwnd, "markallread");
	if (markread)
		HTMLayoutAttachEventHandler(markread, click_markread, (LPVOID)hwnd);

	HELEMENT feedlist = html_getelementbyid (hwnd, "feedlist");
	if (feedlist)
		HTMLayoutAttachEventHandler(feedlist, on_feedlist, (LPVOID)hwnd);

	/* login form */
	HELEMENT loginlink = html_getelementbyid(hwnd, "loginlink");
	if (loginlink)
		HTMLayoutAttachEventHandler(loginlink, click_loginlink, (LPVOID)hwnd);

	HELEMENT closeform = html_getelementbyid(hwnd, "closeform");
	if (closeform)
		HTMLayoutAttachEventHandler(closeform, click_closeform, (LPVOID)hwnd);

	HELEMENT password = html_getelementbyid(hwnd, "password");
	if (password)
		HTMLayoutAttachEventHandler(password, keydown_password, (LPVOID)hwnd);

	HELEMENT board = html_getelementbyid (hwnd, "board");
	if (board)
		HTMLayoutAttachEventHandler(board, hover_board, (LPVOID)hwnd);

	HELEMENT markbtn = html_getelementbyid(hwnd, "markbtn");
	if (markbtn)
		HTMLayoutAttachEventHandler(markbtn, click_markbtn, (LPVOID)hwnd);

	HELEMENT fold = html_getelementbyid(hwnd, "fold");
	if (fold)
		HTMLayoutAttachEventHandler(fold, click_fold, (LPVOID)hwnd);
}


/* 加载提示框, state=1:黄, 0:红 */
void html_showtip(HWND hwnd, const char *info, int state)
{
	HELEMENT tipbox = html_getelementbyid(hwnd, "loadtip");
	if (!tipbox) return;

	const wchar_t *color = state? L"#FFEEB3" : L"#FECFDC";
	HTMLayoutSetStyleAttribute(tipbox, "border-color", color);
	HTMLayoutSetStyleAttribute(tipbox, "background-color", color);
	HTMLayoutSetElementHtml(tipbox, (const BYTE *)info, strlen(info), SIH_REPLACE_CONTENT);
	HTMLayoutSetStyleAttribute(tipbox, "display", L"block");
	HTMLayoutUpdateElement(tipbox, TRUE);
	HTMLayoutUpdateWindow(hwnd);
}

void html_hidetip(HWND hwnd)
{
	HELEMENT tipbox = html_getelementbyid(hwnd, "loadtip");
	if (!tipbox) return;

	HTMLayoutSetStyleAttribute(tipbox, "display", L"none");
}

BOOL html_is_onleft ()
{
	return g_onleft;
}

void html_updatewindow (HWND hwnd)
{
	HELEMENT feedlist = html_getelementbyid(hwnd, "feedlist");
	HTMLayoutUpdateElement(feedlist, TRUE);

	HELEMENT itemlist = html_getelementbyid(hwnd, "itemlist");
	HTMLayoutUpdateElement(itemlist, TRUE);

	HTMLayoutUpdateWindow(hwnd);
}





/**
 * window
 */

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
