#include <process.h>
#include <assert.h>
#include <time.h>
#include <assert.h>
#include "i32.h"
#include "url.h"
#include "xml.h"
#include "db.h"
#include "html.h"

#define must(exp) if(!(exp))return 0


/* 后台下载循环 */
void download (void *param)
{
	printf ("download...\n\n");

	url_download ();

	printf ("i'm done\n");
}

/* 下载并在数据库中插入新的feed, 返回新feed的id */
int insert_newfeed(char *url)
{
	Feed *feed;
	FeedItem *item;
	int id;

	int e;
	char *tmpfile = DOWNLOADDIR"/0.xml";
	int lastupdated;

	url_clear();
	e = url_add (0, url);
	if (!e) return 0;

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

	feed->source = url;
	id = db_insertfeed(feed);
	if (id <= 0) {
		delfeed (feed);
		return -1; /* 源已存在 */
	}

	lastupdated = db_lastupdated (id);
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

int mainform_quit (I32E e)
{
	PostQuitMessage(0);
	return 0;
}

HWND open_mainform()
{
	i32box (NULL, "s|w|h|a|t|bc", WS_OVERLAPPEDWINDOW, 500, 500, "c", TEXT("中铝网阅读器"), -1);
	i32setproc (I32PRE, WM_DESTROY, mainform_quit);

	return I32PRE;
}

int mainform_onsize (I32E e)
{
	i32vfill (e.hwnd,
		i32("htmlbox"), -1,
		NULL);
	return 0;
}

int WINAPI WinMain (HINSTANCE hithis, HINSTANCE hiprev, PSTR param, int icmd)
{
	HWND hwnd = open_mainform();
	i32setproc (hwnd, WM_SIZE, mainform_onsize);

	int e = init_db("subscribe.s3db");
	assert(e);

	db_create_table ();

	//_beginthread (download, 0, NULL);
	//e = insert_feed("http://blog.codingnow.com/atom.xml");
	//if (!e) printf ("insert feed error.\n");

	//e = insert_newfeed("http://rss.follow5.com/rss/user?ai=2261401");
	//if (e <= 0) printf ("insert feed error: %d\n", e);

	//Feed *feed = db_loadfeed (1);
	//if (feed) printf ("title: %\n", feed->updated);

	//int n = update_items(1);
	//printf ("update n: %d\n", n);
	//int lastupdated = db_lastupdated (1);
	//printf ("lastupdated: %d\n", lastupdated);

	reg_html_control();
	//HWND hhtml = i32create (TEXT("html"), "d|s|w|h|a", hwnd, WS_CTRL, 100, 100, "c");
	HWND hhtml = html_create (hwnd, "n|w|h|a", "htmlbox", 200, 200, "c");

	html_loadfile (hhtml, L"demo.htm");

	hhtml = html_create (hwnd, "n|w|h|a", "htmlbox2", 200, 200, "c");
	html_loadstring (hhtml, "<p>sdsf的dd</p>d<p>d<p>d<p>d<p>d<p>d<p>d<p>d<p>");



	ShowWindow (hwnd, SW_SHOW);

	i32loop();
	//close_db();

	return 0;
}
