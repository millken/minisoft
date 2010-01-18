#ifndef _MYCTL_H
#define _MYCTL_H

#include <windows.h>
#include "i32.h"

/* �����б��� */
#define FM_ADDGROUP (WM_USER+1)
#define FM_ADDITEM (WM_USER+2)

/**
 * ����ؼ�����
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


/* ע�������Զ���ؼ� */
void reg_myctl ();


#endif
