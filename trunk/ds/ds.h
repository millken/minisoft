/*
 * c practical data structure
 */

#ifndef _CDS_H
#define _CDS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef __cplusplus
extern "C" {
#endif


/* base type */
#ifndef __cplusplus
typedef enum{false, true} bool;
#endif

#ifndef uint
#define uint unsigned
#endif

#ifndef nil
#define nil NULL
#endif

/* base function */
#ifndef NOMINMAX
#ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
#endif
#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif
#endif

/* memory & string */
void* salloc (size_t size);
#define sfree(p) if(p)free(p) /* safe free */
#define mclean(p,size) memset(p,0,size)
void* mdump(void *data, size_t n); /* memory dump, malloc and memcpy */
char* sdumpn (const char* s, size_t len);
char* sdump(const char *s); /* string dump */
char* fdump (const char* filename); /* load file to string */
char* trimn (const char* text, size_t len);
char* trim (const char* text);

/* string encode */
wchar_t* utf2unicode (char* utf8);

/* hash-table */
typedef struct {
	void* list;
	size_t size;
	uint(*hashfunc)(void*k);
	bool(*cmpfunc)(void*k1,void*k2);
	void*(*kdumpfunc)(void*k);
	void(*kfreefunc)(void*k);
} hst;

hst* hscreat (const char* keytype, size_t size);
void* hsputv (hst* t, void* k, void* v);
void* hsgetv (hst* t, void* k);
void* hsdelv (hst* t, void* k);
void hsclose (hst* t, void(*vfreefunc)(void*));
#define hsput(t,k,v) hsputv(t,(void*)k, (void*)v)
#define hsget(t,k) hsgetv(t,(void*)k)
#define hsdel(t,k) hsdelv(t,(void*)k)


/* variable-length string
 *
 * 在vscat之前要先保证长度正确, 用vsrelen.
 */
typedef char vchar;

void vsinit (size_t tablesize); /* optional */
void vsclose (); /* optional */
vchar* vsdump (const char* s); /* create or copy a new vstring */
void vsfree (vchar* vs);
vchar* vsresize (vchar* vs, size_t size); /* resize, realloc */
void vsrelen (vchar* vs);
vchar* vsncpy (vchar* a, const char*b, size_t len);
#define vscpy(a,b) vsncpy(a,b,strlen(b))
vchar* vsncat (vchar* a, const char* b, size_t n);
#define vscat(a,b) vsncat(a,b,strlen(b))
vchar* vs_vsprintf (vchar* vs, const char* format, __VALIST p);
vchar* vs_sprintf (vchar* vs, const char* format, ...); /* sprintf to a vstring */
vchar* vs_printf (const char* format, ...); /* create a new vstring */
bool isvs (const char* s);
size_t vssize (vchar* vs); /* get mem size */
int vslen (vchar* vs);


/* global list 广义表 */
typedef struct glnode {
	struct glnode *prev, *next;
	struct glnode *child, *dad;
	void* data;
	size_t cn; /* child count */
} glnode;

glnode* glnewv (void* data);
#define glnew(data) glnewv((void*)data)
void glout (glnode* node);
void* gldel (glnode* node, void(*datafreefunc)(void*));
glnode* glget (glnode* dad, int nth); /* get nth child */
size_t glchildn (glnode* dad);
void gljoin (glnode* dad, glnode* node, int nth);
#define glforeach(dad,p) for(p=dad->child;p;p=p->next)



#ifdef __cplusplus
}
#endif

#endif
