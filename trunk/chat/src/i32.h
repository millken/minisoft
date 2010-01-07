#ifndef _I32_H
#define _I32_H

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define I32NAMETABLE_SIZE 16
#define I32MSGTABLE_SIZE 32
#define I32SEP '|'

#define i32pair(a, b) ((POINT){(int)(a), (int)(b)})
#define i32rect(x, y, w, h) ((RECT){(int)(x), (int)(y), (int)(w), (int)(h)})

/* ‘›”√ */
#define i32malloc malloc
#define i32realloc realloc

typedef struct {
	HWND hwnd;
	WPARAM wp;
	LPARAM lp;
} I32EVENT;
typedef int (*I32PROC) (I32EVENT);

void i32bind (HWND hwnd, char *name);
HWND i32 (char *name);
void i32setproc (HWND hwnd, UINT message, I32PROC f);
I32PROC i32getproc (HWND hwnd, UINT message);
void i32set (HWND hwnd, char *format, ...);
HWND i32create (char *classname, char *format, ...);
int i32oldproc (UINT message, I32EVENT e);

int i32msgloop ();
void i32debug ();

#endif
