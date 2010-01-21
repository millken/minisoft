#ifndef _MYCTL_H
#define _MYCTL_H

#include <windows.h>
#include "i32.h"


enum ChatListMsg {
	CLM_SCROLL = WM_USER + 1,
};



void reg_myctl ();  /* 注册所有自定义控件 */


#endif
