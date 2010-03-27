#ifndef _DB_H
#define _DB_H

#include "xml.h"

#ifdef __cplusplus
extern "C" {
#endif


int init_db (char *dbname);
void close_db();
int db_exe (char *sql, ...);
int db_create_table ();
int db_getallfeed (Feed ***feedlist);
int db_insertfeed (Feed *feed);
int db_insertitem (int id, FeedItem *item);
int db_lastupdated (int id);

Feed *db_loadfeed (int id);

#ifdef __cplusplus
}
#endif

#endif
