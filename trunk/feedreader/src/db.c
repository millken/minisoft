#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include "db.h"

#define yes 1
#define no 0
#define must(exp) if(!(exp))return 0
#define slen(s) (s?strlen(s):0)

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


int init_db (char *dbname)
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
	     updated INTEGER DEFAULT 0 \
		)");
}

int db_create_table ()
{
	int e1, e2;

	e1 = db_create_feed();
	e2 = db_create_item();
	return e1 && e2;
}

int db_getallfeed (Feed ***feedlist)
{
	char *err = NULL;
	int e;
	char **result;
	int rown, coln;
	char *format = "select id,title,link,updated from feed";
	int i;
	Feed **list;

	e = sqlite3_get_table (g_db, format, &result, &rown, &coln, &err);
	if (e != SQLITE_OK) {
		printf ("DB-GET ERROR: %s\n", err);
		return 0;
	}

	printf ("count row: %d, coln: %d, title: %s\n", rown, coln, result[coln]);

	if (rown <= 0)
		return 0;

	*feedlist = (Feed **)malloc(sizeof(Feed *) * rown);
	list = *feedlist;
	for (i = 0; i < rown; i++)
		list[i] = newfeed();

	for (i = 0; i < rown; i++) {
		Feed *feed = list[i];
		feed->id = atoi(result[(i+1)*coln]);
		feed->link = result[(i+1)*coln+2];
	}
	sqlite3_free_table (result);

	return rown;
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

int db_lastupdated (int id)
{
	char sql[1024];

	char *err = NULL;
	int e;
	char **result;
	int rown, coln;

	sprintf (sql, "select max(updated) from item where id=%d", id);

	e = sqlite3_get_table (g_db, sql, &result, &rown, &coln, &err);
	if (e != SQLITE_OK) {
		printf ("DB-GET ERROR: %s\n", err);
		return 0;
	}

	printf ("count row: %d, coln: %d, title: %s\n", rown, coln, result[coln]);

	if (rown <= 0)
		return 0;

	printf ("coln: %d\n", coln);
	sqlite3_free_table (result);
	return 1;
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
