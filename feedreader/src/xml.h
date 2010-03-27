#ifndef _PARSEXML_H
#define _PARSEXML_H

#ifdef __cplusplus
extern "C" {
#endif


typedef struct feeditem {
	char *title;
	char *link;
	char *content;
	char *author;
	time_t updated;

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
int parsefeed (char *filename, Feed *feed);
void feedtest (Feed *feed);



#ifdef __cplusplus
}
#endif

#endif
