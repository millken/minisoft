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


typedef int (*I32PROC) (HWND, WPARAM, LPARAM);

void i32bind (HWND hwnd, char *name);
HWND i32h (char *name);
void i32setproc (HWND hwnd, UINT message, I32PROC f);
I32PROC i32getproc (HWND hwnd, UINT message);
void i32set (HWND hwnd, char *format, ...);
HWND i32create (char *classname, char *format, ...);
int i32call_oldproc (HWND hwnd, UINT message, WPARAM wp, LPARAM lp);

int i32msgloop ();
void i32debug ();

#endif
