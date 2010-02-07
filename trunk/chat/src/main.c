#include "i32.h"
#include "ctrls.h"

void reg_form ();
void reg_chatlist ();

void create_dlg (char *name);


void reg_myctl ()
{
	reg_form();
	reg_chatlist();
	reg_butten();
	reg_image();
}

int mainform_onclose (I32E e)
{
	PostQuitMessage(0);

	return 0;
}

int mainform_oncmd (I32E e)
{
	if ((HWND)e.lp == i32("friendlist")) {
		int id = SendMessage (e.lp, CM_GETSELECT, 0, 0);
		if (id > 0) {
			char buf[32];
			itoa(id, buf, 10);
			create_dlg (buf);
		}
	}
	return 0;
}

int mainform_onsize (I32E e)
{
	i32vfill ( e.hwnd,
		i32("panel-t"), 90,
		i32("friendlist"), -1,
		i32("panel-b"), 28,
		NULL
	);
	return 0;
}

int panelt_onsize (I32E e)
{
	int w = i32clientw(e.hwnd);

	i32set(i32id(e.hwnd, 103), "x|y", 4, 8);
	i32set(i32id(e.hwnd, 101), "x|y", 5, 30);
	i32set(i32id(e.hwnd, 102), "a|-x|+y", "t|r", 7, 14);

	i32set(i32("searchbox"), "w|a|-y", w-6, "c|b", 4);

	return 0;
}

int panelt_lbuttondown (I32E e)
{
	i32set(i32("signediter"), "w|show", 0, "no");
	i32set(i32("signbutton"), "w", 100);
	return 0;
}

int signbutton_lbuttonup (I32E e)
{
	RECT sr;
	HWND signbutton = i32("signeditor");

	i32dadrect (e.hwnd, &sr);
	i32set(e.hwnd, "w", 0);
	i32set (signbutton, "x|y|w|show", sr.left, sr.top+2, 120, "y");

	return 0;
}

int panelb_onsize (I32E e)
{
	HWND htab[] = {
		i32id(e.hwnd, 101),
		i32id(e.hwnd, 102),
		i32id(e.hwnd, 103),
		i32id(e.hwnd, 104),
		};

	i32hfill (e.hwnd,
		htab[0], 0,
		htab[1], 0,
		htab[2], 0,
		htab[3], 0,
		NULL);

	return 0;
}

int mainform_onwheel (I32E e)
{
	SendMessage (i32("friendlist"), e.msg, e.wp, e.lp);
	return 0;
}

int mainform_onfocus (I32E e)
{
	SetFocus(i32("friendlist"));
	return -1;
}

void create_form ()
{
	int i;

	HWND hbutten, himage, hmyname, htab[4];
	HWND hclist;
	HWND hwnd = i32create (TEXT("form"), "n|t|s|w|h|a|+x|-y|bc", "mainform", TEXT("財神"),
				WS_OVERLAPPEDWINDOW
				, 270, 500, "l|b", 8, 80, -1);

	i32setproc (hwnd, WM_DESTROY, mainform_onclose);
	i32setproc (hwnd, WM_SIZE, mainform_onsize);
	i32setproc (hwnd, WM_COMMAND, mainform_oncmd);
	i32setproc (hwnd, WM_MOUSEWHEEL, mainform_onwheel);
	i32setproc (hwnd, WM_SETFOCUS, mainform_onfocus);

	i32box ("panel-t", hwnd);
	i32set (i32("panel-t"), "bc", 0xEE9C59);
	i32setproc (i32("panel-t"), WM_SIZE, panelt_onsize);
	i32setproc (i32("panel-t"), WM_LBUTTONDOWN, panelt_lbuttondown);

	i32box ("panel-b", hwnd);
	i32set (i32("panel-b"), "bc", RGB(229, 239, 254));
	i32setproc (i32("panel-b"), WM_SIZE, panelb_onsize);

	i32create (TEXT("chatlist"), "d|n|s|w|h|a|show", hwnd, "friendlist",
				WS_CTRL|WS_VSCROLL, 100, 100, "c", "y");

	hbutten = i32create(TEXT("butten"), "d|n|t|s|id|w|h|a|f|show", i32("panel-t"), "signbutton", TEXT("지 못한哈哈Эучены"),
				WS_CTRL, 101, 140, 22, "c|t", "Verdana,15", "y");
	SendMessage (hbutten, BM_SETBCOLOR, RGB(89,156,238), 0);
	SendMessage (hbutten, BM_SETFCOLOR, 0xeeeeee, 0);
	SendMessage (hbutten, BM_SETBCOLOR_HOVER, RGB(160,199,245), 0);
	SendMessage (hbutten, BM_SETBCOLOR_PUSHED, RGB(160,199,245), 0);
	SendMessage (hbutten, BM_SETRADIUS, 0, 0);
	SendMessage (hbutten, BM_SETMARGIN, 5, 0);
	SendMessage (hbutten, BM_SETTEXTCOLOR, 0x565656, 0);
	SendMessage (hbutten, BM_SETALIGN, 0, 0);
	i32setproc (hbutten, WM_LBUTTONUP, signbutton_lbuttonup);
	//SendMessage (hbutten, BM_SETICON, LoadBitmap(GetModuleHandle(0), TEXT("NAME_FLAG")), 0);

	himage = i32create(TEXT("image"), "d|s|id|w|h", i32("panel-t"), WS_CTRL, 102, 37, 37);
	SendMessage (himage, IM_SETIMAGE, LoadBitmap(GetModuleHandle(0), TEXT("AVATAR_B")), 0);
	SendMessage (himage, IM_SETFCOLOR_HOVER, 0x555555, 0);

	hmyname = i32create(TEXT("butten"), "s|d|t|id|w|h|f|show",  WS_CTRL, i32("panel-t"), TEXT("Cat Studio"),
			103, 100, 24, "Verdana,15", "y");
	SendMessage (hmyname, BM_SETMARGIN, 5, 0);
	SendMessage (hmyname, BM_SETICONALIGN, 1, 0);
	SendMessage (hmyname, BM_SETICON, LoadBitmap(GetModuleHandle(0), TEXT("NAME_FLAG")), 0);
	SendMessage (hmyname, BM_SETALIGN, 0, 0);

	/* 签名框 */
	i32create (TEXT("edit"), "n|s|d|w|h|show", "signeditor", WS_CTRL, i32("panel-t"), 100, 22, "no");
	/* 搜索框 */
	i32create (TEXT("edit"), "n|s|d|h|f", "searchbox", WS_CTRL|WS_BORDER, i32("panel-t"), 22, "Verdana,15");

	/* tabs */
	htab[0] = i32create(TEXT("butten"), "s|d|id|t|w|h|f", WS_CTRL, i32("panel-b"), 101, TEXT("好友"), 44, 20, "Verdana,15,1");
	htab[1] = i32create(TEXT("butten"), "s|d|id|t|w|h|f", WS_CTRL, i32("panel-b"), 102, TEXT("群组"), 44, 20, "Verdana,15,1");
	htab[2] = i32create(TEXT("butten"), "s|d|id|t|w|h|f", WS_CTRL, i32("panel-b"), 103, TEXT("最近联系人"), 80, 20, "Verdana,15,1");
	htab[3] = i32create(TEXT("butten"), "s|d|id|t|w|h|f", WS_CTRL, i32("panel-b"), 104, TEXT("网盘"), 40, 20, "Verdana,15,1");
	for (i = 0; i < 4; i++) {
		SendMessage (htab[i], BM_SETBCOLOR, RGB(229, 239, 254), 0);
		SendMessage (htab[i], BM_SETBCOLOR_HOVER, 0xffffff, 0);
		SendMessage (htab[i], BM_SETBCOLOR_PUSHED, 0xffffff, 0);
		SendMessage (htab[i], BM_SETFCOLOR, 0xffffff, 0);
	}

	ShowWindow (hwnd, SW_SHOW);
}




int dlg_onclose (I32E e)
{
	//PostQuitMessage(0);
	return -1;
}

int dlg_onsize (I32E e)
{
	HWND hpanel_top = i32id(e.hwnd, 1000);
	HWND hpanel_mid = i32id(e.hwnd, 2000);
	HWND hpanel_foot = i32id(e.hwnd, 3000);

	i32vfill (e.hwnd,
		hpanel_top, 40,
		hpanel_mid, -1,
		hpanel_foot, 40,
		NULL);
	return 0;
}

int dlgpnt_onsize (I32E e)
{
	i32set(i32id(e.hwnd, 100), "a|x", "c", 10);
	i32set(i32id(e.hwnd, 200), "a|x", "c", 90);
	i32set(i32id(e.hwnd, 300), "a|x", "c", 160);

	return 0;
}

void create_dlg (char *name)
{
	HWND hwnd, hpanel, hbutten[3];
	int i;
	if (!name || i32(name)) return;

	hwnd = i32create(TEXT("form"), "n|t|s|w|h|a|x|y|bc", name,
			TEXT("对话 Dog"), WS_OVERLAPPEDWINDOW, rand()%350+100, rand()%350+40,
			"l|b", rand()%500, rand()%500, -1);
	i32setproc (hwnd, WM_SIZE, dlg_onsize);
	i32setproc (hwnd, WM_CLOSE, dlg_onclose);

	/* top panel */
	hpanel = i32create(TEXT("box"), "s|d|id|bc", WS_CTRL, hwnd, 1000, 0xEE9C59);
	i32setproc (hpanel, WM_SIZE, dlgpnt_onsize);
	hbutten[0] = i32create(TEXT("butten"), "s|d|id|t|w|h|f|show", WS_CTRL, hpanel, 100,
				TEXT("传送文件"), 80, 22, "Arial,15", "y");
	hbutten[1] = i32create(TEXT("butten"), "s|d|id|t|w|h|f|show", WS_CTRL, hpanel, 200,
				TEXT("视频"), 64, 22, "Arial,15", "y");
	hbutten[2] = i32create(TEXT("butten"), "s|d|id|t|w|h|f|show", WS_CTRL, hpanel, 300,
				TEXT("聊天记录"), 80, 22, "Arial,15", "y");
	for (i = 0; i < 3; i++) {
		SendMessage (hbutten[i], BM_SETICON, LoadBitmap(GetModuleHandle(0), TEXT("ICON_FILE")), 0);
		SendMessage (hbutten[i], BM_SETBCOLOR, 0xEE9C59, 0);
		SendMessage (hbutten[i], BM_SETBCOLOR_HOVER, RGB(160,199,245), 0);
		SendMessage (hbutten[i], BM_SETBCOLOR_PUSHED, RGB(160,199,245), 0);
		SendMessage (hbutten[i], BM_SETFCOLOR, 0xeeeeee, 0);
		SendMessage (hbutten[i], BM_SETRADIUS, 2, 0);
	}

	/* middle panel */
	i32create(TEXT("box"), "s|d|id|bc", WS_CTRL, hwnd, 2000, 0xffffff);
	/* foot panel */
	i32create(TEXT("box"), "s|d|id|bc", WS_CTRL, hwnd, 3000, 0xEE9C59);

	ShowWindow (hwnd, SW_SHOW);
}



void create_richedit ()
{


}

int WINAPI WinMain (HINSTANCE hithis, HINSTANCE hiold, PSTR param, int cmd)
{
	reg_myctl ();

	create_form ();
	create_dlg ("11004");


	i32loop();
	return 0;
}
