/*
 *   i32: GUI模块
 */

#ifndef _I32_H
#define _I32_H

#undef WINVER
#define WINVER 0x0500

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif

#define I32NAMETABLE_SIZE 128
#define I32MSGTABLE_SIZE 512
#define I32ATTRTABLE_SIZE 128

#define I32DOT '|'
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

/* 绘图 */
void i32framerect (HDC hdc, RECT *r, DWORD col);
void i32fillrect (HDC hdc, RECT *r, DWORD col);
int i32clientw (HWND hwnd);
int i32clienth (HWND hwnd);
void i32bltbmp (HDC hdc, HBITMAP hbmp, int x, int y);

/* 其他 */
void i32bind (HWND hwnd, char *name);
I32PROC i32getproc (HWND hwnd, UINT message);
void i32debug ();


#ifdef __cplusplus
}
#endif

#endif
