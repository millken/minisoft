#include <stdlib.h>
#include <io.h>
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

static CRITICAL_SECTION g_cs; /* url_download同步锁 */
static int g_isactive = 0;
static HANDLE g_mutex = NULL;

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
	char tmpfile[256];
	sprintf (tmpfile, "%s/0.xml", url_get_downdir());
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
	char tmpfile[256];
	sprintf (tmpfile, "%s/0.xml", url_get_downdir());
	e = parsefeed(tmpfile, feed);
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

HWND open_mainform()
{
	HWND hwnd, hhtml;

	reg_form();
	//hwnd = i32box (NULL, "s|w|h|a|t|bc", WS_OVERLAPPEDWINDOW, 800, 600, "c", TEXT("FeedReader"), -1);
	hwnd = i32create (TEXT("form"), "n|s|w|h|a|t|bc", "mainform",
		WS_OVERLAPPEDWINDOW, 800, 600, "c", TEXT("FeedReader"), -1);

	reg_html_control();
	hhtml = html_create (hwnd, "n|w|h|a", "htmlbox", 200, 200, "c");
	html_loadfile (hhtml, L"theme/index.htm");

	return hwnd;
}

void quit_mainform (HWND hwnd)
{
	tydel (hwnd);
	PostQuitMessage (0);
	CloseHandle(g_mutex);
	exit(0); /* 这样快 */
}


int mainform_syscmd (I32E e)
{
	if (e.wp == SC_CLOSE) {
		ShowWindow(e.hwnd, SW_HIDE);
		return 0;
	}
	return -1;
}

int mainform_onkey (I32E e)
{
	if (e.wp == VK_ESCAPE) {
		ShowWindow (i32("mainform"), SW_HIDE);
		return 0;
	}

	return -1;
}

int mainform_onsyschar (I32E e)
{
	if (e.wp==(int)'Q' || e.wp==(int)'q') {
		quit_mainform (i32("mainform"));
		return 0;
	}

	return -1;
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
	if (g_isactive)
		html_refresh_feedlist(i32("htmlbox"));

	return -1;
}

void popmenu (HWND hwnd)
{
	POINT pt;
	HMENU hmenu;

	/* 设置为前台窗口,以在单击非弹出菜单区域时使菜单自动消失! */
	SetForegroundWindow (hwnd);
	hmenu = LoadMenu (GetModuleHandle(NULL), TEXT("tray_menu"));
	hmenu = GetSubMenu (hmenu, 0);

	GetCursorPos (&pt);
	TrackPopupMenu (hmenu, TPM_RIGHTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y,
											 0, hwnd, NULL);
}

int mainform_ontray (I32E e)
{
	if (e.wp==ID_TRAY && e.lp==WM_LBUTTONUP) {
		if (!IsWindowVisible(e.hwnd)) {
			ShowWindow(e.hwnd, SW_MINIMIZE); /* 有时候在任务栏里不出来 */
			i32send(e.hwnd, WM_SYSCOMMAND, SC_RESTORE, 0);
			SetFocus(i32("htmlbox"));
		}
		else {
			ShowWindow(e.hwnd, SW_HIDE);
		}
	}
	else if (e.wp==ID_TRAY && e.lp==WM_RBUTTONUP) {
		popmenu (e.hwnd);
	}
	else if (e.wp==ID_TRAY && e.lp==WM_RBUTTONDBLCLK) { /* 右键双击退出 */
		quit_mainform (e.hwnd);
	}

	return 0;
}

int mainform_oncmd (I32E e)
{
	switch (LOWORD(e.wp)) {
		case 101:
			quit_mainform (e.hwnd);
		break;

		case 100:
			ShowWindow (e.hwnd, SW_SHOW);
		break;
	}
	return 0;
}

void CALLBACK timerproc (HWND hwnd, UINT a, UINT b, DWORD d)
{
	HWND hhtml;
	wchar_t buf[128];

	int unreadn = db_unreadcount(0);
	if (unreadn > 0)
		tymod (hwnd, TEXT("tray_unread"));
	else
		tymod (hwnd, TEXT("tray_ico"));

	wsprintfW(buf, L"%d条未读", unreadn);
	tytip (hwnd, buf);

	if ( unreadn>0 && g_isactive && !html_is_onleft() ) {
		HWND hhtml = i32("htmlbox");
		html_refresh_feedlist (hhtml);
		html_updatewindow (hhtml);
	}
}


int WINAPI WinMain (HINSTANCE hithis, HINSTANCE hiprev, PSTR param, int icmd)
{
	/* 建立用户个人目录 */
	int r = url_set_userdir(param);
	while (!r) {
		int e = MessageBox(NULL, TEXT("程序没有写权限!"), TEXT("发生异常"), MB_ABORTRETRYIGNORE);
		if (e == IDABORT)
			return 0;
		else if (e == IDRETRY) {
			r = url_set_userdir(param);
			continue;
		}
		else if (e == IDIGNORE)
			break;
		else
			return 0;
	}


	/* 每用户只需运行一个实例 */
	HANDLE g_mutex = CreateMutexA(FALSE, NULL, url_get_userdir());
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		CloseHandle(g_mutex);
		MessageBox(NULL,
			TEXT("程序已经在运行中, 不能重复启动!"), TEXT("提示"), MB_OK|MB_ICONWARNING);
		return 0;
	}


	HWND hwnd = open_mainform();

	i32setproc (hwnd, WM_SYSCOMMAND, mainform_syscmd);
	i32setproc (hwnd, WM_SIZE, mainform_onsize);
	i32setproc (hwnd, WM_NCACTIVATE, mainform_onact);
	tyadd (hwnd, TEXT("tray_ico"), TEXT("Feed Reader"));
	i32setproc (hwnd, ON_TRAY, mainform_ontray);
	i32setproc (hwnd, WM_COMMAND, mainform_oncmd);

	SetTimer(hwnd, 1, 2000, timerproc); /* 检查新feed */

	char dbpath[128];
	sprintf (dbpath, "%s/feed.db", url_get_userdir());
	int e = init_db(dbpath);
	assert(e);

	db_create_table ();

	InitializeCriticalSection (&g_cs);
	_beginthread (download, 0, NULL); /* 开始后台循环抓取 */


	HWND hhtml = i32("htmlbox");
	html_loadfeedlist (hhtml);
	html_loaditemlist (hhtml, 0);
	i32setproc (hhtml, WM_KEYDOWN, mainform_onkey);
	i32setproc (hwnd, WM_KEYDOWN, mainform_onkey);
	i32setproc (hhtml, WM_SYSCHAR, mainform_onsyschar);
	i32setproc (hwnd, WM_SYSCHAR, mainform_onsyschar);
	html_set_subevt (hhtml, click_sub); /* click one feed */

	ShowWindow (hwnd, SW_SHOW);
	SetFocus(hhtml);

	/* end */
	i32loop();

	DeleteCriticalSection (&g_cs) ;
	close_db();
	CloseHandle(g_mutex);

	return 0;
}
