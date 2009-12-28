#ifndef _I32_H
#define _I32_H

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NAMETABLE_SIZE 128
#define MSGTABLE_SIZE 1

// ‘›”√
#define i32malloc malloc
#define i32realloc realloc

typedef int (*I32CALLBACK) (HWND, WPARAM, LPARAM);

void i32bind (HWND hwnd, char *name);
HWND i32h (char *name);
HWND i32create (char *classname, char *name);
void i32set_callback (HWND hwnd, UINT message, I32CALLBACK f);
I32CALLBACK i32get_callback (HWND hwnd, UINT message);



#endif
