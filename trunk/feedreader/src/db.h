#ifndef _DB_H
#define _DB_H

#include "xml.h"
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif


int init_db (const char *dbname);
void close_db();
int db_exe (char *sql, ...);
int db_create_table ();

int db_insertfeed (Feed *feed);
int db_insertitem (int id, FeedItem *item);
int db_newupdated (int feedid);

Feed *db_loadfeed (int id);
FeedItem *db_loaditem (int itemid);

/* 设置已读状态 */
void db_markread (int itemid, int state);
int db_unreadcount (int feedid);

/* 查询集合 */
int db_select_feedlist (Feed ***feedlist, const char *where, ...);
void db_del_feedlist (Feed **feedlist, int rown);

int db_select_itemlist (FeedItem ***itemlist, const char *where, ...);
void db_del_itemlist (FeedItem **feeditem, int rown);

#ifdef __cplusplus
}
#endif

#endif
