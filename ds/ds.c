/*
 * c practical data structure
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <windows.h>

#include "ds.h"

/*
 * misc
 */
#define _assert(exp) if(!(exp))exit(1371)
#define log(msg) fprintf(stderr, "err: %s\n", msg)/* TODO */

/*
 * memory
 */
void *salloc (size_t size)
{
	void *p = malloc(size);
	if (!p) log("malloc fail");
	memset(p, 0, size);
	return p;
}

void* mdump (void* data, size_t n)
{
	void* p = salloc(n);
	if (data)
		memcpy(p, data, n);
	return p;
}

char* sdumpn (const char* s, size_t len)
{
	char *p;

	s = s ? s : "";
	if (!s) len = 0;
	p = (char*)salloc(len+1);
	strncpy(p, s, len);
	p[len] = '\0';
	return p;
}

char* sdump (const char* s)
{
	char *p;
	int len;

	s = s ? s : "";
	len = strlen(s);
	p = (char*)salloc(len+1);
	strncpy(p, s, len);
	p[len] = '\0';
	return p;
}

char* fdump (const char* filename)
{
	FILE* f;
	int size, n;
	char* buffer;

	f = fopen(filename, "rb");
	if (!f) {
		log("cant open file.");
		return NULL;
	}

	fseek (f, 0L, SEEK_END);
	size = ftell (f);
	fseek (f, 0L, SEEK_SET);

	buffer = (char*)salloc(size+1);
	if (!buffer) {
		fclose(f);
		return NULL;
	}

	n = fread (buffer, size, 1, f);
	if (n != 1) {
		free (buffer);
		fclose (f);
		return NULL;
	}

	buffer[size] = '\0';
	fclose(f);
	return buffer;
}

char* trimn (const char* text, size_t len)
{
	char *begin, *end;

	for (begin = (char*)text; isspace(*begin); begin++);
	for (end = (char*)(text+len); end>begin && isspace(end[-1]); end--);

	return sdumpn(begin, end-begin);
}

char* trim (const char* text)
{
	return trimn(text, strlen(text));
}

wchar_t* utf2unicode (char* utf8)
{
	wchar_t* ws;
	size_t n;

	n = MultiByteToWideChar (CP_UTF8, 0, utf8, -1, NULL, 0);
	ws = (wchar_t*)salloc((n+1)*sizeof(wchar_t));
	MultiByteToWideChar (CP_UTF8, 0, utf8, -1, ws, sizeof(wchar_t)*(n+1));
	return ws;
}

/*
 * hash-table
 */
typedef struct _hsnode {
	struct _hsnode* next;
	void* k;
	void* v;
} hsnode;

static uint hshashstr (void *k)
{
	register uint code = 5381;
	char *s = (char*)k;

	if (s == NULL) return 0;
	while (*s)
		code += (code << 5) + (*s++);

	return code;
}

static bool hscmpstr (void *k1, void *k2)
{
	return strcmp((const char*)k1, (const char*)k2) == 0;
}

static void* hsdumpstr (void *k)
{
	return (void*)sdump((const char*)k);
}

static void hsfreestr (void *k)
{
	sfree(k);
}

static uint hsptrhash (void* k)
{
	register uint c = 5371;
	register int i;
	unsigned char* buf = (unsigned char*)&k;

	for (i = 0; i < 4; i++)
		c += (c<<5) + buf[i];

	return c;
}

hst* hscreat (const char* keytype, size_t size)
{
	hst* t = (hst*)salloc(sizeof(hst));
	t->size = size;
	t->list = salloc(sizeof(hsnode*) * size);

	if (keytype && strcmp(keytype, "ptr")==0) {
		t->hashfunc = hsptrhash;
	}
	else if (keytype && strcmp(keytype, "string")==0) {
		t->hashfunc = hshashstr;
		t->cmpfunc = hscmpstr;
		t->kdumpfunc = hsdumpstr;
		t->kfreefunc = hsfreestr;
	}

	return t;
}

void* hsgetv (hst* t, void* k)
{
	hsnode **lp, *p;
	hsnode** list = (hsnode**)t->list;
	uint code;
	bool r;

	code = t->hashfunc ? t->hashfunc(k) : (uint)k;
	lp = &list[code%t->size];
	while (*lp) {
		p = *lp;
		r = t->cmpfunc ? t->cmpfunc(p->k, k) : p->k==k;
		if (r)
			return p->v;
		lp = &p->next;
	}
	return NULL;
}


void* hsputv (hst* t, void* k, void* v)
{
	hsnode **lp, *p;
	hsnode** list = (hsnode**)t->list;
	uint code;
	bool r;

	code = t->hashfunc ? t->hashfunc(k) : (uint)k;
	lp = &list[code%t->size];
	while (*lp) {
		p = *lp;
		r = t->cmpfunc ? t->cmpfunc(p->k, k) : p->k==k;
		if (r) {
			void *oldv = p->v;
			p->v = v;
			return oldv;
		}
		lp = &p->next;
	}

	p = *lp = (hsnode*)salloc(sizeof(hsnode));
	p->k = t->kdumpfunc ? t->kdumpfunc(k) : k;
	p->v = v;

	return NULL;
}

void* hsdelv (hst* t, void* k)
{
	hsnode **lp, *p, *next;
	hsnode** list = (hsnode**)t->list;
	uint code;
	bool r;

	code = t->hashfunc ? t->hashfunc(k) : (uint)k;
	lp = &list[code%t->size];
	while (*lp) {
		p = *lp;
		r = t->cmpfunc ? t->cmpfunc(p->k, k) : p->k==k;
		if (r) {
			void *oldv = p->v;
			next = p->next;
			if (t->kfreefunc)
				t->kfreefunc(p->k);
			sfree(p);
			*lp = next;
			return oldv;
		}
		lp = &p->next;
	}

	return NULL;
}

void hsclose (hst* t, void(*vfreefunc)(void*))
{
	hsnode *p, *next;
	hsnode** list;
	uint i;

	if (!t) return;
int n = 0;
	list = (hsnode**)t->list;
	for (i = 0; i < t->size; i++) {
		p = list[i];
		if(p) n++;
		while (p) {
			next = p->next;
			if (t->kfreefunc)
				t->kfreefunc(p->k);
			if (vfreefunc)
				vfreefunc(p->v);
			sfree(p);
			p = next;
		}
	}
printf("%d\n", n);
	sfree(t->list);
	sfree(t);
}


/*
 * vstring
 */
typedef struct {
	size_t len;
	size_t size;
} vs_data;

static hst* g_vst = NULL;

void vsinit (size_t tablesize)
{
	if (g_vst) return;
	g_vst = hscreat("ptr", tablesize);
}

static void _vsinit ()
{
	vsinit(1024); /* default max table size */
}

static void _vsfreecb (void *p)
{
	sfree(p);
}

void vsclose ()
{
	hsclose(g_vst, _vsfreecb);
	g_vst = NULL;
}


vchar* vsdump (const char* s)
{
	char* vs;
	vs_data *d;

	_vsinit();
	vs = sdump(s);
	d = (vs_data*)salloc(sizeof(vs_data));
	d->len = strlen(vs);
	d->size = d->len+1;
	hsput(g_vst, vs, d);

	return vs;
}

void vsfree (vchar* vs)
{
	vs_data* d = hsdel(g_vst, vs);
	sfree(d);
	sfree(vs);
}

vchar* vsresize (vchar* vs, size_t size)
{
	vchar* p;
	vs_data* d;

	p = (vchar*)realloc(vs, size);

	if (p != vs) {
		d = (vs_data*)hsdel(g_vst, vs);
		d->size = size;
		hsput(g_vst, p, d);
	}
	else {
		d = (vs_data*)hsget(g_vst, vs);
		d->size = size;
	}

	return p;
}

void vsrelen (vchar* vs)
{
	vs_data* d = (vs_data*)hsget(g_vst, vs);
	if (d)
		d->len = strlen(vs);
}

bool isvs (const char* s)
{
	return hsget(g_vst, s)!=NULL;
}

size_t vssize (vchar* vs)
{
	vs_data* d = (vs_data*)hsget(g_vst, vs);
	return d ? d->size : 0;
}

int vslen (vchar* vs)
{
	vs_data* d = (vs_data*)hsget(g_vst, vs);
	return d ? d->len : strlen(vs);
}

vchar* vsncpy (vchar* a, const char*b, size_t len)
{
	vs_data* d = (vs_data*)hsget(g_vst, a);
	if (d->size < len+1)
		a = vsresize(a, (len+5)*1.2);
	d->len = len;
	strncpy (a, b, len);
	a[len] = '\0';
	return a;
}

vchar* vsncat (vchar* a, const char* b, size_t n)
{
	int alen;
	size_t size;

	vs_data* d = (vs_data*)hsget(g_vst, a);
	alen = d->len;
	size = d->size;
	d->len = alen + n; /* new length */

	if (d->len+1 > size) {
		size = (d->len+2)*1.5;
		a = vsresize (a, size);
	}

	strncpy (a+alen, b, n);
	a[d->len] = '\0';
	return a;
}

vchar* vs_vsprintf (vchar* vs, const char* format, __VALIST p)
{
	vs_data* d;

	if (!vs) vs = vsdump(NULL);

	d = (vs_data*)hsget(g_vst, vs);
	d->len = vsnprintf(NULL, 0, format, p);
	if (d->len+1 > d->size)
		vs = vsresize(vs, (d->len+5)*1.2);

	vsprintf(vs, format, p);
	return vs;
}

vchar* vs_sprintf (vchar*vs, const char* format, ...)
{
	va_list p;

	va_start(p, format);
	vs = vs_vsprintf(vs, format, p);
	va_end(p);

	return vs;
}

vchar* vs_printf (const char* format, ...)
{
	va_list p;
	vchar* vs = vsdump(NULL);

	va_start(p, format);
	vs = vs_vsprintf(vs, format, p);
	va_end(p);

	return vs;
}

/*
 * global list 广义表
 */
glnode* glnewv (void* data)
{
	glnode* n = (glnode*)salloc(sizeof(glnode));
	n->data = data;
	return n;
}

void glout (glnode* node)
{
	glnode *dad, *next, *prev;

	if (!node) return;

	dad = node->dad;
	next = node->next;
	prev = node->prev;

	if (dad && dad->child == node) {
		dad->child = next ? next : prev;
		dad->cn--;
	}

	if (next)
		next->prev = prev;
	if (prev)
		prev->next = next;

	node->dad = NULL;
	node->prev = NULL;
	node->next = NULL;
}

void* gldel (glnode* node, void(*freedatafunc)(void*))
{
	glnode *p, *next;
	void* data;

	if (!node) return NULL;

	data = node->data;

	/* del children */
	for (p = node->child; p; ) {
		next = p->next;
		gldel(p, freedatafunc);
		p = next;
	}

	/* del self */
	glout (node);
	if (freedatafunc) {
		freedatafunc (node->data);
		data = NULL;
	}
	free (node);

	return data;
}

/* insert after one */
static void glinsert (glnode* dst, glnode* src)
{
	glnode* next;

	if (!dst || !src) return;

	next = dst->next;
	dst->next = src;
	src->next = next;
	src->prev = dst;
	if (next)
		next->prev = src;
}

static void glinsertbefore (glnode* dst, glnode* src)
{
	glnode* prev;

	if (!dst || !src) return;

	prev = dst->prev;
	src->next = dst;
	src->prev = prev;
	dst->prev = src;
	if (prev)
		prev->next = src;
}

size_t glchildn (glnode* dad)
{
	glnode *p;
	size_t n = 0;

	if (!dad->child) return 0;

	for (p = dad->child; p; p = p->next)
		n++;
	return n;
}

glnode* glget (glnode* dad, int nth)
{
	glnode *p;
	int i;

	if (!dad->child) return NULL;

	if (nth < 0) {
		nth += dad->cn + 1;
		if (nth <= 0) return NULL;
	}

	for (i = 1, p=dad->child; i < nth && p->next; i++, p=p->next);

	return p;
}

void gljoin (glnode* dad, glnode* node, int nth)
{
	glnode *p;

	if (!dad || !node) return;

	p = nth!=0 ? glget(dad, nth) : NULL;
	if (p) {
		glinsert(p, node);
	}
	else {
		if (dad->child) {
			glinsertbefore(dad->child, node);
		}
		else {
			node->next = NULL;
			node->prev = NULL;
		}
		dad->child = node;
	}
	node->dad = dad;
	dad->cn++;
}

