#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include "db.h"

#define yes 1
#define no 0
#define must(exp) if(!(exp))return 0
#define slen(s) (s?strlen(s):0)
#define mfree(p) if(p)free(p)

static sqlite3 *g_db = NULL;


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


int init_db (const char *dbname)
{
	int e;

	must(dbname);
	e = sqlite3_open(dbname, &g_db);

	return !e;
}

void close_db()
{
	sqlite3_close(g_db);
}

/* 失败返回0, 成功返回非0 */
int db_exe (char *sql, ...)
{
	char *err;
	int e;
	char *zsql;
	va_list p;

	va_start(p, sql);
	zsql = sqlite3_vmprintf(sql, p);
	va_end(p);

	e = sqlite3_exec(g_db, zsql, 0, 0, &err);
	if (e) printf("DB ERROR: %s\n", err);
	sqlite3_free(zsql);

	return e == SQLITE_OK;
}

static int db_create_feed ()
{
	return db_exe ("CREATE TABLE feed( \
	     id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, \
	     source VARCHAR(2048) UNIQUE NULL, \
	     title VARCHAR(1024), \
	     link VARCHAR(2048), \
	     updated INTEGER DEFAULT 0 \
		)");
}

static int db_create_item ()
{
	return db_exe ("CREATE TABLE item( \
	     id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, \
	     feedid INTEGER NOT NULL, \
	     title VARCHAR(1024), \
	     link VARCHAR(2048), \
	     content TEXT, \
	     author VARCHAR(512), \
	     updated INTEGER DEFAULT 0, \
	     read INTEGER DEFAULT 0 \
		)");
}

int db_create_table ()
{
	int e1, e2;

	e1 = db_create_feed();
	e2 = db_create_item();
	return e1 && e2;
}

int db_select_feedlist (Feed ***feedlist, const char *where, ...)
{
	char *pwhere, *sql;
	va_list vp;

	char *err = NULL;
	int e;
	char **result;
	int rown, coln;
	int i;
	Feed **list;

	must(feedlist);

	va_start (vp, where);
	pwhere = sqlite3_vmprintf (where, vp);
	va_end (vp);
	sql = sqlite3_mprintf ("select id,updated,source,title,link from feed where %s", pwhere);
	sqlite3_free (pwhere);

	e = sqlite3_get_table (g_db, sql, &result, &rown, &coln, &err);
	sqlite3_free (sql);
	if (e != SQLITE_OK) {
		printf ("DB SELECT FEED ERROR: %s\n", err);
		return 0;
	}

	must (rown > 0);

	*feedlist = (Feed **)malloc(sizeof(Feed *) * rown);
	list = *feedlist;
	for (i = 0; i < rown; i++)
		list[i] = newfeed();

	for (i = 0; i < rown; i++) {
		Feed *feed = list[i];
		feed->id = atoi(result[(i+1)*coln]);
		feed->updated = atoi(result[(i+1)*coln+1]);
		feed->source = dump(result[(i+1)*coln+2]);
		feed->title = dump(result[(i+1)*coln+3]);
		feed->link = dump(result[(i+1)*coln+4]);
	}
	sqlite3_free_table (result);

	return rown;
}

void db_del_feedlist (Feed **feedlist, int rown)
{
	int i;

	if (rown <= 0) return;

	for (i = 0; i < rown; i++) {
		Feed *feed = feedlist[i];
		delfeed (feed);
	}
	mfree (feedlist);
}

/* 给定where条件查询一个item集合 */
int db_select_itemlist (FeedItem ***itemlist, const char *where, ...)
{
	char *pwhere, *sql;
	va_list vp;

	char *err = NULL;
	int e;
	char **result;
	int i, rown, coln;
	FeedItem **list;

	must(itemlist);

	va_start (vp, where);
	pwhere = sqlite3_vmprintf (where, vp);
	va_end (vp);
	sql = sqlite3_mprintf ("select id,feedid,updated,title,link,content,author,read from item where %s", pwhere);
	sqlite3_free (pwhere);

	e = sqlite3_get_table (g_db, sql, &result, &rown, &coln, &err);
	sqlite3_free (sql);
	if (e != SQLITE_OK) {
		printf ("SELECT ITEM ERROR: %s\n", err);
		return 0;
	}

	must(rown > 0);

	*itemlist = (FeedItem **)malloc(sizeof(FeedItem *) * rown);
	list = *itemlist;

	for (i = 0; i < rown; i++) {
		FeedItem *item = list[i];
		list[i] = newitem();
		item = list[i];
		item->id = atoi(result[(i+1)*coln]);
		item->feedid = atoi(result[(i+1)*coln+1]);
		item->updated = atoi(result[(i+1)*coln+2]);
		item->title = dump(result[(i+1)*coln+3]);
		item->link = dump(result[(i+1)*coln+4]);
		item->content = dump(result[(i+1)*coln+5]);
		item->author = dump(result[(i+1)*coln+6]);
		item->read = atoi(result[(i+1)*coln+7]);
	}

	sqlite3_free_table (result);

	return rown;
}

void db_del_itemlist (FeedItem **feeditem, int rown)
{
	int i;

	for (i = 0; i < rown; i++)
		delitem(feeditem[i]);
	mfree (feeditem);
}


int db_insertfeed (Feed *feed)
{
	char *format;

	must(feed);

	format = "insert into feed(source, title, link, updated) values('%q', '%q', '%q', %d)";

	if (!db_exe(format, feed->source, feed->title, feed->link, feed->updated))
		return 0;

	return (int)sqlite3_last_insert_rowid(g_db);
}


int db_insertitem (int id, FeedItem *item)
{
	must(item);

	return db_exe ("insert into item(feedid, title, link, content, author, updated) \
				values(%d, '%q', '%q', '%q', '%q', %d)",
			id, item->title, item->link, item->content, item->author, item->updated);
}

int db_newupdated (int feedid)
{
	char *sql;
	sqlite3_stmt *st;
	int e;
	int updated = 0;

	sql = sqlite3_mprintf ("select max(updated) from item where feedid=%d", feedid);
	sqlite3_prepare (g_db, sql, -1, &st, 0);
	sqlite3_free (sql);

	e = sqlite3_step(st);
	if (e == SQLITE_ROW)
		updated = sqlite3_column_int(st, 0);

	sqlite3_finalize (st);
	return updated;
}

/* 只取feed不取item */
Feed *db_loadfeed (int id)
{
	char *sql;
	sqlite3_stmt *st;
	int e;
	Feed *feed;

	feed = newfeed();
	must (feed);

	sql = sqlite3_mprintf ("select id,source,title,link from feed where id=%d limit 1", id);
	sqlite3_prepare(g_db, sql, -1, &st, 0);
	sqlite3_free(sql);

	e = sqlite3_step(st);
	if (e != SQLITE_ROW) return NULL;

	//while (e == SQLITE_ROW) {
	feed->id = sqlite3_column_int(st, 0);
	feed->source = dump((char *)sqlite3_column_text(st, 1));
	feed->title = dump((char *)sqlite3_column_text(st, 2));
	feed->link = dump((char *)sqlite3_column_text(st, 3));

	/* 取最新更新时间 */
	feed->updated = db_newupdated (id);

	sqlite3_finalize(st);

	return feed;
}


/* 通用获取单条item */
FeedItem *db_loaditem_sql (char *sql)
{
	sqlite3_stmt *st;
	int e;
	FeedItem *item;

	must(sql);

	item = newitem();
	must(item);

	sqlite3_prepare(g_db, sql, -1, &st, 0);

	e = sqlite3_step(st);
	must(e == SQLITE_ROW);

	item->id = sqlite3_column_int(st, 0);
	item->feedid = sqlite3_column_int(st, 1);
	item->title = dump((char *)sqlite3_column_text(st, 2));
	item->link = dump((char *)sqlite3_column_text(st, 3));
	item->content = dump((char *)sqlite3_column_text(st, 4));
	item->author = dump((char *)sqlite3_column_text(st, 5));
	item->updated = sqlite3_column_int(st, 6);
	item->read = sqlite3_column_int(st, 7);

	sqlite3_finalize(st);

	return item;
}

/* 加载一条指定id的item */
FeedItem *db_loaditem (int itemid)
{
	char *sql;
	FeedItem *item;

	sql = sqlite3_mprintf ("select id,feedid,title,link,content,author,updated,read from item where id=%d limit 1", itemid);
	item = db_loaditem_sql(sql);
	sqlite3_free(sql);

	return item;
}

/* 标记某条为已读 */
void db_markread (int itemid, int state)
{
	db_exe("update item set read=%d where id=%d", state, itemid);
}

/* 某feed未读条数 */
int db_unreadcount (int feedid)
{
	char *sql, *format;
	sqlite3_stmt *st;
	int e;
	int n = 0;

	if (feedid == 0)
		format = "select count(*) from item where read=0";
	else
		format = "select count(*) from item where feedid=%d and read=0";

	sql = sqlite3_mprintf (format, feedid);
	sqlite3_prepare(g_db, sql, -1, &st, 0);
	sqlite3_free(sql);

	e = sqlite3_step(st);
	if (e != SQLITE_ROW) return 0;

	n = sqlite3_column_int(st, 0);

	sqlite3_finalize(st);

	return n;
}
