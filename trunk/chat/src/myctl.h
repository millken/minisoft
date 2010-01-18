#ifndef _MYCTL_H
#define _MYCTL_H

#include <windows.h>
#include "i32.h"

/* 分组列表常量 */
#define FM_ADDGROUP (WM_USER+1)
#define FM_ADDITEM (WM_USER+2)

/**
 * 分组控件类型
 */
typedef struct group {
	unsigned gid;
	char *name;
} Group;

typedef struct item {
	unsigned id;
	BOOL onfocus;
	BOOL onhover;
	unsigned gid;
	char *title;
	char *ftitle;
	char *sign;
	HBITMAP thumb;
} Item;


/* 注册所有自定义控件 */
void reg_myctl ();


#endif
