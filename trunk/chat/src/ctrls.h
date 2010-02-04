#ifndef _CTRLS_H
#define _CTRLS_H

/* chatlist控件的消息 */
enum CHATLIST_MSG {

	CM_SETVIEWMODE = WM_USER+1,
	CM_GETVIEWMODE,
	CM_ADDGROUP,
	CM_SETGROUP_NAME,
	CM_SETGROUP_NOTE,
	CM_DELGROUP,

	CM_ADDBUDDY,
	CM_SETBUDDY_NAME,
	CM_SETBUDDY_NOTE,
	CM_SETBUDDY_SIGN,
	CM_SETBUDDY_PIC,
	CM_SETBUDDY_STATUS,
	CM_DELBUDDY,

	CM_BLSORT /* 单组好友列表重新排序 */
};

/* 按钮控件消息 */
enum {
	BM_SETRADIUS = WM_USER + 1,
	BM_SETMARGIN,
	BM_SETTITLEMARGIN,
	BM_SETBCOLOR,
	BM_SETBCOLOR_HOVER,
	BM_SETBCOLOR_PUSHED,
	BM_SETFCOLOR,
	BM_SETTEXTCOLOR,

	BM_LBUTTONDOWN,
	BM_LBUTTONUP,
	BM_RBUTTONDOWN,
	BM_RBUTTONUP
};

/* 图片控件消息 */
enum {
	IM_SETIMAGE = WM_USER + 1,
	IM_SETPADDING,
	IM_SETBCOLOR,
	IM_SETBCOLOR_HOVER,
	IM_SETFCOLOR,
	IM_SETFCOLOR_HOVER
};


#endif