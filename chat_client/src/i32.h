/*
 *   i32: GUI模块
 *   工程最好预定义UNICODE, 需要msimg32.lib
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
#define i32send(h,m,wp,lp) SendMessage((HWND)h, m, (WPARAM)wp, (LPARAM)lp)
#define I32PRE (i32pre())

typedef struct {
	HWND hwnd;
	UINT msg;
	WPARAM wp;
	LPARAM lp;
} I32EVENT, I32E;

typedef int (*I32PROC) (I32EVENT);

/* 核心 */
HWND i32create (TCHAR *classname, char *format, ...);
HWND i32pre (); /* 取得上一个创建的hwnd */
void i32setpre (HWND hwnd);
void i32set (HWND hwnd, char *format, ...);
HWND i32 (char *name);
void i32setproc (HWND hwnd, UINT message, I32PROC f);
int i32callold (I32EVENT e);
int i32loop ();

/* 布局 */
HWND i32box (HWND dad, char *format, ...);
void i32vfill (HWND hwnd, ...);
void i32hfill (HWND hwnd, ...);

/* GDI */
#define i32loadbmp(rcname) LoadBitmap(GetModuleHandle(0), rcname)
void i32dadrect (HWND hwnd, RECT *r);
void i32mousepos (HWND hwnd, POINT *p); /* 获得光标位置 */
void i32framerect (HDC hdc, RECT *r, DWORD col);
void i32fillrect (HDC hdc, RECT *r, DWORD col);
void i32line (HDC hdc, int x, int y, int tox, int toy, DWORD col);
int i32clientx (HWND hwnd);
int i32clienty (HWND hwnd);
int i32clientw (HWND hwnd);
int i32clienth (HWND hwnd);
void i32blt (HDC hdc, HBITMAP hbmp, int x, int y);
void i32draw (HDC hdc, HBITMAP hbmp, int x, int y, int neww, int newh);
void i32hblt (HDC hdc, HBITMAP hbmp, int x, int y, int index, int pagen);
void i32vblt (HDC hdc, HBITMAP hbmp, int x, int y, int index, int pagen);
void i32textout (HDC hdc, int x, int y, TCHAR *text, DWORD col);

/* 其他 */
void i32bind (HWND hwnd, char *name);
I32PROC i32getproc (HWND hwnd, UINT message);
void i32debug ();
void i32vset (HWND hwnd, char *format, va_list p);
void i32error (char *format, ...);

/* 常用控件 */
HWND i32static (HWND dad, char *format, ...);
HWND i32edit (HWND dad, char *format, ...);
HWND i32pwdedit (HWND dad, char *format, ...);
HWND i32checkbox (HWND dad, char *format, ...);
inline BOOL i32getcheck (HWND hcheckbox);
inline void i32setcheck (HWND hcheckbox, BOOL v);

#ifdef __cplusplus
}
#endif

#endif
