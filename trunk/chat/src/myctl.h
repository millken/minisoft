#ifndef _MYCTL_H
#define _MYCTL_H

#include <windows.h>
#include "i32.h"

/**
 * 分组控件消息
 */
enum glist_messages {
	GLM_ADDGROUP = WM_USER+1,  /* 新加组 */
	GLM_DELGROUP,  /* 删除组 */
	GLM_RENAMEGROUP,  /* 更改组名 */

	GLM_ADDITEM,
	GLM_DELITEM,
	GLM_GETITEM,
	GLM_SETITEM_GID,
};

/**
 * 分组控件类型
 */
typedef struct group {
	int gid;
	char *name;
} Group;

typedef struct item {
	int id;
	int status;
	BOOL onfocus;
	BOOL onhover;
	int gid;
	char *title;
	char *title2;
	char *title3;
	HBITMAP thumb;
} Item;


/* 注册所有自定义控件 */
void reg_myctl ();


#endif
