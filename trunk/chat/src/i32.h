/*
 *   i32: GUI模块
 */

#ifndef _I32_H
#define _I32_H

#undef WINVER
#define WINVER 0x0500

#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif

#define WS_CTRL (WS_CHILD|WS_VISIBLE)

/* 暂用 */
#define i32malloc malloc
#define i32realloc realloc
#define i32free(p) do{if(p)free(p);}while(0)

#define i32id GetDlgItem

typedef struct {
	HWND hwnd;
	UINT msg;
	WPARAM wp;
	LPARAM lp;
} I32EVENT, I32E;

typedef int (*I32PROC) (I32EVENT);


/* 核心 */
HWND i32create (TCHAR *classname, char *format, ...);
void i32set (HWND hwnd, char *format, ...);
HWND i32 (char *name);
void i32setproc (HWND hwnd, UINT message, I32PROC f);
int i32callold (I32EVENT e);
int i32loop ();

/* 布局 */
HWND i32box (char *name, HWND dad);
void i32vfill (HWND hwnd, ...);
void i32hfill (HWND hwnd, ...);

/* GDI */
void i32dadrect (HWND hwnd, RECT *r);
void i32mousepos (HWND hwnd, POINT *p); /* 获得光标位置 */
void i32framerect (HDC hdc, RECT *r, DWORD col);
void i32fillrect (HDC hdc, RECT *r, DWORD col);
void i32line (HDC hdc, int x, int y, int tox, int toy, DWORD col);
int i32clientw (HWND hwnd);
int i32clienth (HWND hwnd);
void i32blt (HDC hdc, HBITMAP hbmp, int x, int y);
void i32draw (HDC hdc, HBITMAP hbmp, int x, int y, int neww, int newh);
void i32hblt (HDC hdc, HBITMAP hbmp, int x, int y, int index, int pagen);
void i32textout (HDC hdc, int x, int y, TCHAR *text, DWORD col);

/* 其他 */
void i32bind (HWND hwnd, char *name);
I32PROC i32getproc (HWND hwnd, UINT message);
void i32debug ();
void i32vset (HWND hwnd, char *format, va_list p);

#ifdef __cplusplus
}
#endif

#endif
