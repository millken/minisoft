#ifndef _I32_H
#define _I32_H

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NAMETABLE_SIZE 16
#define MSGTABLE_SIZE 32

// ‘›”√
#define i32malloc malloc
#define i32realloc realloc

typedef int (*I32PROC) (HWND, WPARAM, LPARAM);

void i32bind (HWND hwnd, char *name);
HWND i32h (char *name);
HWND i32create (char *classname, char *name);
void i32setproc (HWND hwnd, UINT message, I32PROC f);
I32PROC i32getproc (HWND hwnd, UINT message);

void i32debug ();

#endif
