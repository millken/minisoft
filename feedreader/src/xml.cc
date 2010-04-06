#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tinyxml.h>
#include <iconv.h>
#include <time.h>
#include "xml.h"

#define yes 1
#define no 0
#define null NULL

#define mfree(p) if(p)free(p)
#define sure(exp) if(!(exp)){printf("Error Line: %d\n", __LINE__);return 0;}  /* 断言 */
#define streq(a,b) (!strcmp(a, b))  /* 判断字符串相同 */


FeedItem *newitem ()
{
	FeedItem *item = (FeedItem *)malloc(sizeof(FeedItem));
	memset(item, 0, sizeof(FeedItem));
	return item;
}

void delitem (FeedItem *item)
{
	if (!item) return;

	mfree (item->author);
	mfree (item->link);
	mfree (item->title);
	mfree (item->content);
	mfree (item);
}


Feed *newfeed ()
{
	Feed *feed = (Feed *)malloc(sizeof(Feed));
	memset(feed, 0, sizeof(Feed));
	return feed;
}

void delfeed (Feed *feed)
{
	FeedItem *item;
	if (!feed) return;

	mfree (feed->source);
	mfree (feed->title);
	mfree (feed->link);

	for (item = feed->list; item; ) {
		FeedItem *next = item->next;
		delitem (item);
		item = next;
	}

	memset (feed, 0, sizeof(Feed));
}


static char *encode (iconv_t cv, const char *in)
{
	if (!in) return NULL;

	size_t inlen = strlen(in);
	size_t outlen = inlen*2 + 1;

	char *out = (char *)malloc(outlen);
	memset(out, 0, outlen);

	char *outp = out;
	int e = iconv(cv, &in, &inlen, &outp, &outlen);
	if (e) out[0] = '\0';

	return out;
}

static char *strcopy (const char *in)
{
	int inlen = in ? strlen(in) : 0;
	char *out = (char *)malloc(inlen+1);
	if (in)
		memcpy(out, in, inlen);
	out[inlen] = '\0';
	return out;
}

static time_t mktime_atom (const char *time)
{
	struct tm ts;
	time_t t = 0;
	int e = 0, n;
	float sec;
	int zoneh, zonem;
	char sign;

	sure(time);

	memset(&ts, 0, sizeof ts);
	ts.tm_isdst = 0;

	n = sscanf (time, "%d-%d-%dT%d:%d:%f%c%d:%d",
		&ts.tm_year, &ts.tm_mon, &ts.tm_mday, &ts.tm_hour, &ts.tm_min, &sec, &sign, &zoneh, &zonem);
	if (n == 9) {
		ts.tm_sec = (int)sec;
		if (sign == '-')
			zoneh = -zoneh;
		ts.tm_year -= 1900;
		ts.tm_mon -= 1;
		t = mktime(&ts);
		if (t > -1) {
			t += zoneh*3600;
			e = 1;
		}
	}

	if (!e) {
		n = sscanf (time, "%d-%d-%dT%d:%d:%dZ",
			&ts.tm_year, &ts.tm_mon, &ts.tm_mday, &ts.tm_hour, &ts.tm_min, &ts.tm_sec);
		if (n == 6) {
			ts.tm_year -= 1900;
			ts.tm_mon -= 1;
			t = mktime(&ts);
			if (t > -1)
				e = 1;
		}
	}

	return e ? t : 0;
}

/* Sat, 26 Dec 2009 10:35:00 +0800 */
static time_t rss_timeformat1 (const char *format, struct tm *ts)
{
	static char *month[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	                          "Jul", "Aug", "Sept", "Oct", "Nov", "Dec"};
	time_t t;
	int n;
	int zone = 0;
	char sign, comma;
	char zonebuf[10];
	char weekbuf[4], monbuf[5];
	int i;

	n = sscanf (format, "%3s%c %d %4s %d %d:%d:%d %s",
		weekbuf, &comma, &ts->tm_mday, monbuf, &ts->tm_year, &ts->tm_hour, &ts->tm_min, &ts->tm_sec, zonebuf);
	if (n < 8) return 0;

	/* month */
	for (i = 0; i < 12; i++)
		if (strcmp(month[i], monbuf) == 0)
			break;
	if (i >= 12) return 0;
	ts->tm_mon = i;

	ts->tm_year -= 1900;

	/* parse zone */
	if (n == 9) {
		n = sscanf (zonebuf, "%c%4d", &sign, &zone);
		if (n == 2 && (sign=='+' || sign=='-')) {
			zone /= 100;
			if (sign == '-')
				zone = -zone;
		}
	}

	t = mktime(ts);
	if (t == -1)
		return 0;

	t += zone * 3600;
	return t;
}

/* 2010-03-21 12:45:00.0 */
static time_t rss_timeformat2 (const char *format, struct tm *ts)
{
	time_t t;
	float sec;
	int n;

	n = sscanf (format, "%4d-%2d-%2d %2d:%2d:%f",
		&ts->tm_year, &ts->tm_mon, &ts->tm_mday, &ts->tm_hour, &ts->tm_min, &sec);
	if (n < 6) return 0;

	ts->tm_year -= 1900;
	ts->tm_mon--;
	ts->tm_sec = (int)sec;

	t = mktime(ts);
	if (t == -1)
		return 0;

	return t;
}

static time_t mktime_rss (const char *format)
{
	struct tm ts;
	time_t t = 0;

	sure(format);

	memset (&ts, 0, sizeof ts);
	ts.tm_isdst = 1;

	t = rss_timeformat1(format, &ts);
	if (!t)
		t = rss_timeformat2(format, &ts);

	return t;
}

static int parse_atom (iconv_t cv, TiXmlNode *head, Feed *feed)
{
	printf ("atom..\n");
	sure(head);
	sure(feed);

	// top node: <feed>
	TiXmlElement *elem;

	// <title>
	sure( elem = head->FirstChildElement("title") );
	feed->title = encode(cv, elem->GetText());
	//printf ("%s: %s\n", elem->Value(), feed->title);

	// <link>
	elem = head->FirstChildElement("link");
	feed->link = elem ? encode(cv,elem->Attribute("href")) : strcopy("");
	//printf ("link:%s\n", feed->link);

	// <updated>
	elem = head->FirstChildElement("updated");
	feed->updated = elem ? mktime_atom(elem->GetText()) : 0;
	//printf ("updated: %s\n", ctime(&feed->updated));

	// <entry> list
	FeedItem **lp = &feed->list;
	for (elem = head->FirstChildElement("entry"); elem; elem = elem->NextSiblingElement("entry"))
	{
		FeedItem *item = newitem();
		TiXmlElement *sub;

		// <title>
		sub = elem->FirstChildElement("title");
		item->title = sub ? encode(cv,sub->GetText()) : strcopy("");
		//printf ("  title:%s\n", item->title);

		// <link>
		sub = elem->FirstChildElement("link");
		item->link = sub ? encode(cv,sub->Attribute("href")) : strcopy("");
		//printf ("  link:%s\n", item->link);

		// <updated>
		sub = elem->FirstChildElement("updated");
		item->updated = sub ? mktime_atom(sub->GetText()) : 0;
		//printf ("time: %s\n", ctime(&item->updated));

		// <content>
		sub = elem->FirstChildElement("content");
		item->content = sub ? encode(cv,sub->GetText()) : strcopy("");
		//printf ("  content: %s\n", item->content);

		// <author><name>
		sub = elem->FirstChildElement("author");
		if (sub)
			sub = sub->FirstChildElement("name");
		item->author = sub ? encode(cv,sub->GetText()) : strcopy("");
		//printf ("  author: %s\n", item->author);

		*lp = item;
		lp = &item->next;
	}

	return yes;
}

static int parse_rss (iconv_t cv, TiXmlNode *head, Feed *feed)
{
	printf ("rss..\n");

	sure(head);
	sure(feed);

	// <rss>
	TiXmlElement *elem;

	// <channel>
	head = head->FirstChildElement("channel");
	sure(head);

	// <title>
	elem = head->FirstChildElement("title");
	sure(elem);
	feed->title = encode(cv, elem->GetText());
	//printf ("title: %s\n", feed->title);

	// <link>
	elem = head->FirstChildElement("link");
	feed->link = elem ? encode(cv,elem->GetText()) : strcopy("");
	//printf ("link: %s\n", feed->link);

	// <pubDate>
	elem = head->FirstChildElement("pubDate");
	feed->updated = elem ? mktime_rss(elem->GetText()) : 0;
	//printf ("pubdate: %s\n", ctime(&feed->updated));

	// <item> list
	FeedItem **lp = &feed->list;
	for (elem = head->FirstChildElement("item"); elem; elem=elem->NextSiblingElement("item"))
	{
		TiXmlElement *sub;
		FeedItem *item = newitem();

		// <title>
		sub = elem->FirstChildElement("title");
		item->title = sub ? encode(cv, sub->GetText()) : strcopy("");
		//printf ("title: %s\n", item->title);

		// <pubDate>
		sub = elem->FirstChildElement("pubDate");
		item->updated = sub ? mktime_rss(sub->GetText()) : 0;
		//printf ("update: %s\n", ctime(&item->updated));

		// <link>
		sub = elem->FirstChildElement("link");
		item->link = sub ? encode(cv, sub->GetText()) : strcopy("");
		//printf ("link: %s\n", item->link);

		// <description> or <content:encoded>
		sub = elem->FirstChildElement("description");
		const char *desc = sub ? sub->GetText() : null;
		int dlen = desc ? strlen(desc) : 0;

		sub = elem->FirstChildElement("content:encoded");
		const char *content = sub ? sub->GetText() : null;
		int clen = content ? strlen(content) : 0;

		const char *s = clen>=dlen ? content : desc;

		item->content = s ? encode(cv, s) : strcopy("");
		//printf ("content: %s\n", item->content);

		// <dc:creator>
		sub = elem->FirstChildElement("dc:creator");
		item->author = sub ? encode(cv, sub->GetText()) : strcopy("");
		//printf ("author: %s\n", item->author);

		*lp = item;
		lp = &item->next;
	}

	return yes;
}

int parsefeed (char *filename, Feed *feed)
{
	sure(feed);

	TiXmlDocument doc;
	TiXmlDeclaration *decl;
	TiXmlNode *node, *first;

	const char *code;
	iconv_t cv;

	sure( doc.LoadFile(filename) );

	sure( node = doc.FirstChild() );

	/* 第一行必须是声明 */
	sure( decl = node->ToDeclaration() );
	code = decl->Encoding();
	sure( !streq(code, "") );
	//printf ("encode: %s\n", code);

	/* 确定编码 */
	cv = iconv_open ("utf-8", code);
	if (cv == (iconv_t)-1) return no;

	/* Atom or RSS */
	int e = no;
	if (first = node->NextSibling("feed"))
		e = parse_atom (cv, first, feed);
	else
	if (first = node->NextSibling("rss"))
		e = parse_rss (cv, first, feed);

	iconv_close (cv);

	return e;
}


void feedtest (Feed *feed)
{
	FeedItem *item;

	printf ("Feed Title: %s\n", feed->title);
	printf ("Feed Link: %s\n", feed->link);
	printf ("Feed Updated: %s", feed->updated?ctime(&feed->updated):"0");
	printf ("Feed List:\n");

	for (item = feed->list; item; item=item->next) {
		printf ("Item Title: %s\n", item->title);
		printf ("Item Link: %s\n", item->link);
		printf ("Item Updated: %s\n", item->updated?ctime(&item->updated):"0");
		printf ("Item Author: %s\n", item->author);
		printf ("Item Content: %s\n", item->content);
	}
}
