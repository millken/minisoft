/*
 * 各种窗口(主窗口|登录窗口|对话框|设置框 ...)
 *
 * 界面与逻辑的混合模块,相当于控制器
 */

#ifndef _WINS_H
#define _WINS_H

/*
 * 主窗口 MainForm
 */

/* tab pages id */
enum {
	ID_LOGPAD = 1,
	ID_LOGEDPAD
};

void create_mainform ();
void mainform_set_toppad (int padid);



/*
 * 单人对话框
 */
HWND create_chatview (int uid);

/*
 * 多人对话框
 */
HWND create_groupview (int gid);


#endif
