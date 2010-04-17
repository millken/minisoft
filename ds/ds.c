/*
 * c practical data structure
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

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

vchar* vs_vsprintf (vchar* vs, const char* format, va_list p)
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
glnode* glnew (void* data)
{
	glnode* n = (glnode*)salloc(sizeof(glnode));
	n->data = data;
	return n;
}

void glleave (glnode* node)
{
	glnode *dad, *next, *prev;

	if (!node) return;

	dad = node->dad;
	next = node->next;
	prev = node->prev;

	if (dad) {
		dad->child = next;
		if (next) next->dad = dad;
	}

	if (next)
		next->prev = prev;
	if (prev)
		prev->next = next;
}

void* gldel (glnode* node, void(*freedatafunc)(void*))
{
	glnode *p;
	void* data;

	if (!node) return NULL;

	data = node->data;

	/* del children */
	for (p = node->child; p; p=p->next)
		gldel(p, freedatafunc);

	/* del self */
	glleave (node);
	if (freedatafunc) {
		freedatafunc (node->data);
		data = NULL;
	}
	free (node);

	return data;
}

/* insert after one */
void glinsert (glnode* dst, glnode* src)
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

size_t glchildn (glnode* dad)
{
	glnode *p, *begin, *end;
	size_t n = 0;

	if (!dad->child) return 0;

	begin = dad->child;
	end = begin->prev;
	n = 1;
	for (p=begin; p!=end; p=p->next)
		n++;
	return n;
}

glnode* glchild (glnode* dad, int nth)
{
	glnode *p, *begin, *end;
	int i;

	if (!dad->child) return NULL;

	begin = dad->child;
	end = begin->prev;

	if (nth == 0) nth = 1;
	if (nth > 0)
		for (i=1, p=begin; i<=nth && p!=end; i++, p=p->next);
	return p;
}

void* glchildv (glnode* dad, int nth)
{
	glnode* p = glchild(dad, nth);
	return p ? p->data : NULL;
}

/* insert to be the last child */
void gljoin (glnode* dad, glnode* node, int nth)
{
	glnode *p, *old;

	if (!dad || !node) return;

	p = nth!=0 ? glchild(dad, nth) : NULL;
	if (p) {
		glinsert(p, node);
	}
	else {
		old = dad->child;
		dad->child = node;
		node->dad = dad;
		if (old) {
			printf ("y");
			node->next = old;
			node->prev = old->prev;
			old->prev = node;
			old->dad = NULL;
		}
		else {
			printf ("n");
			node->next = node;
			node->prev = node;
		}
	}
}
