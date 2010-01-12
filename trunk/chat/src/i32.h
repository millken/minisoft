#ifndef _I32_H
#define _I32_H

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define I32NAMETABLE_SIZE 16
#define I32MSGTABLE_SIZE 32
#define I32DOT '|'

/* 暂用 */
#define i32malloc malloc
#define i32realloc realloc

#define i(name) i32(#name)

typedef struct {
	HWND hwnd;
	WPARAM wp;
	LPARAM lp;
} I32EVENT;

typedef int (*I32PROC) (I32EVENT);


/* 核心 */
HWND i32create (char *classname, char *format, ...);
void i32set (HWND hwnd, char *format, ...);
HWND i32 (char *name);
void i32setproc (HWND hwnd, UINT message, I32PROC f);
int i32loop ();

/* 布局 */
HWND i32box (char *name, HWND dad);
void i32vfill (HWND hwnd, ...);
void i32hfill (HWND hwnd, ...);

/* 其他 */
void i32bind (HWND hwnd, char *name);
int i32oldproc (UINT message, I32EVENT e);
I32PROC i32getproc (HWND hwnd, UINT message);
void i32debug ();


#endif
