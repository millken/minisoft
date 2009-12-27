#ifndef _I32_H
#define _I32_H

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NAMETABLE_SIZE 2

// ‘›”√
#define i32malloc malloc

void i32bind (HWND hwnd, char *name);
HWND i32h (char *name);
HWND i32create (char *classname, char *name);

#endif
