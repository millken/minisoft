#ifndef _PARSEXML_H
#define _PARSEXML_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct feeditem {
	int id;
	int feedid;
	time_t updated;

	char *title;
	char *link;
	char *content;
	char *author;

	int read; /* 0:未读, 1:已读 */

	struct feeditem *next;
} FeedItem;

typedef struct {
	int id;
	char *source;

	char *title;
	char *link;
	time_t updated;
	FeedItem *list;
} Feed;


Feed *newfeed ();
void delfeed (Feed *feed);
FeedItem *newitem ();
void delitem (FeedItem *item);
int parsefeed (char *filename, Feed *feed);
void feedtest (Feed *feed);



#ifdef __cplusplus
}
#endif

#endif
