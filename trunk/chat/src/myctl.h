#ifndef _MYCTL_H
#define _MYCTL_H

#include <windows.h>
#include "i32.h"

/**
 * ����ؼ���Ϣ
 */
enum glist_messages {
	GLM_ADDGROUP = WM_USER+1,  /* �¼��� */
	GLM_DELGROUP,  /* ɾ���� */
	GLM_RENAMEGROUP,  /* �������� */

	GLM_ADDITEM,
	GLM_DELITEM,
	GLM_GETITEM,
	GLM_SETITEM_GID,
};

/**
 * ����ؼ�����
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


/* ע�������Զ���ؼ� */
void reg_myctl ();


#endif
