#include <process.h>
#include <assert.h>
#include <time.h>
#include <assert.h>
#include "i32.h"
#include "url.h"
#include "xml.h"
#include "db.h"
#include "html.h"
#include "ctrls.h"
#include "tray.h"

#define must(exp) if(!(exp))return 0

static CRITICAL_SECTION g_cs;
static int g_isactive = 0;

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


/* 下载并在数据库中插入新的feed和item, 返回新feed的id */
int insert_newfeed(char *url)
{
	Feed *feed;
	FeedItem *item;
	int id = 0;

	int e;
	char *tmpfile = (char *)DOWNLOADDIR"/0.xml";
	int lastupdated = 0;

	url_clear();
	e = url_add (0, url); if (!e) return 0;
	url_download ();
	url_clear();

	feed = newfeed();
	e = parsefeed (tmpfile, feed);
	if (!e) {
		printf ("load feed error.\n");
		delfeed (feed);
		return 0;
	}
	printf ("feed title: %s\n", feed->title);

	feed->source = dump(url);
	id = db_insertfeed(feed);
	if (id <= 0) {
		delfeed (feed);
		return -1; /* 源已存在 */
	}

	lastupdated = db_newupdated (id);
	printf ("lastupdated: %d\n", lastupdated);

	for (item = feed->list; item; item=item->next)
		if (item->updated > 0 && item->updated > lastupdated)
			db_insertitem (id, item);

	delfeed(feed);
	printf ("finished.\n");
	printf ("id: %u\n", id);
	return id;
}

/* 更新单个博客的所有文章,返回新的文章数 */
int update_items (int feedid)
{
	Feed *feed;
	FeedItem *item;
	int newtime;
	int e, n = 0;

	must(feedid > 0);

	/* download */
	feed = db_loadfeed(feedid);
	must(feed);

	newtime = feed->updated;

	url_clear();
	url_add (0, feed->source);
	url_download ();
	url_clear();

	delfeed(feed);

	/* parse */
	feed = newfeed();
	e = parsefeed(DOWNLOADDIR"/0.xml", feed);
	if (e) {
		for (item = feed->list; item; item=item->next)
			if (item->updated > newtime) {
				n += db_insertitem (feedid, item) != 0;
			}
	}
	delfeed(feed);

	return n;
}

/* 新订阅 */
static void click_sub (HWND hwnd, char *url)
{
	int sud = db_feedexist(url);
	if (sud) {
		html_showtip (hwnd, "你已经订阅过此博客", 1);
		return;
	}

	//printf ("url: %s#\n", url);
	html_showtip (hwnd, "正在抓取..", 1);

	EnterCriticalSection (&g_cs);

	int e = insert_newfeed(url);
	if (!e) printf ("insert feed error.\n");

	LeaveCriticalSection (&g_cs);

	if (e > 0)
		html_hidetip (hwnd);
	else if (e < 0)
		html_showtip (hwnd, "你已经订阅过此博客", 1);
	else
		html_showtip (hwnd, "噢, 发生错误!", 0);

	if (e > 0) {
		html_clearfeedlist(hwnd);
		html_loadfeedlist(hwnd);
	}
}

/* 后台抓取 */
void download (void *param)
{
	int feedid = 0;

	while (1) {
		EnterCriticalSection (&g_cs);

		feedid = db_nextfeedid(feedid);
		printf ("grabing: %d\n", feedid);
		if (feedid > 0)
			update_items(feedid);

		LeaveCriticalSection (&g_cs);
		Sleep(2000);
	}
}

/**
 * UI
 */
int mainform_quit (I32E e)
{
	tydel (e.hwnd);
	PostQuitMessage(0);
	exit(0); /* 这样快 */
	return 0;
}

HWND open_mainform()
{
	HWND hwnd, hhtml;

	reg_form();
	//hwnd = i32box (NULL, "s|w|h|a|t|bc", WS_OVERLAPPEDWINDOW, 800, 600, "c", TEXT("FeedReader"), -1);
	hwnd = i32create (TEXT("form"), "s|w|h|a|t|bc",
		WS_OVERLAPPEDWINDOW, 800, 600, "c", TEXT("FeedReader"), -1);
	i32setproc (I32PRE, WM_DESTROY, mainform_quit);

	reg_html_control();
	hhtml = html_create (hwnd, "n|w|h|a", "htmlbox", 200, 200, "c");
	html_loadfile (hhtml, L"theme/index.htm");

	return hwnd;
}

int mainform_onsize (I32E e)
{
	i32vfill (e.hwnd,
		i32("htmlbox"), -1,
		NULL);
	return 0;
}

int mainform_onact (I32E e)
{
	g_isactive = LOWORD(e.wp);
	printf ("%d\n", g_isactive);
	if (g_isactive)
		html_refresh_feedlist(i32("htmlbox"));

	return 0;
}

void popmenu (HWND hwnd)
{
	POINT pt;
	HMENU hmenu;

	/* 设置为前台窗口,以在单击非弹出菜单区域时使菜单自动消失! */
	SetForegroundWindow (hwnd);
	hmenu = LoadMenu (GetModuleHandle(NULL), "POPMENU");
	hmenu = GetSubMenu (hmenu, 0);

	GetCursorPos (&pt);
	TrackPopupMenu (hmenu, TPM_RIGHTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y,
											 0, hwnd, NULL);
}

int mainform_ontray (I32E e)
{
	if (e.wp==ID_TRAY && e.lp==WM_LBUTTONDOWN) {
		ShowWindow (e.hwnd, SW_SHOW);
		SetFocus(i32("htmlbox"));
	}
	else
	if (wp==ID_TRAY && lp==WM_RBUTTONUP) {

	}

	return 0;
}

void CALLBACK timerproc (HWND hwnd, UINT a, UINT b, DWORD d)
{
	HWND hhtml;

	if (!html_is_onleft() && g_isactive) {
		HWND hhtml = i32("htmlbox");
		html_refresh_feedlist (hhtml);
		html_updatewindow (hhtml);
	}
}

int WINAPI WinMain (HINSTANCE hithis, HINSTANCE hiprev, PSTR param, int icmd)
{
	HWND hwnd = open_mainform();
	i32setproc (hwnd, WM_SIZE, mainform_onsize);
	i32setproc (hwnd, WM_NCACTIVATE, mainform_onact);
	SetTimer(hwnd, 1, 1000, timerproc); /* 检查新feed */
	tyadd (hwnd, L"tray_ico", L"Feed Reader");
	i32setproc (hwnd, ON_TRAY, mainform_ontray);

	int e = init_db("feed.db");
	assert(e);

	db_create_table ();

	InitializeCriticalSection (&g_cs);
	_beginthread (download, 0, NULL); /* 开始后台循环抓取 */


	HWND hhtml = i32("htmlbox");
	html_loadfeedlist (hhtml);
	html_loaditemlist (hhtml, 0);

	html_set_subevt (hhtml, click_sub); /* click one feed */

	ShowWindow (hwnd, SW_MINIMIZE); /* 加载延时 */
	ShowWindow (hwnd, SW_SHOWNORMAL);
	SetFocus(hhtml);

	/* end */
	i32loop();

	DeleteCriticalSection (&g_cs) ;
	close_db();

	return 0;
}
