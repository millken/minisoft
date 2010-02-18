#include "i32.h"
#include "ctrls.h"

void reg_form ();
void reg_chatlist ();

void create_dlg (char *name);


char *pagename = "friendlist"; /* 选中了哪个tabpage */

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
	if (HIWORD(e.wp)==CM_LUP && (HWND)e.lp == i32("friendlist")) {
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
	ShowWindow (i32("friendlist"), SW_HIDE);
	ShowWindow (i32("grouplist"), SW_HIDE);
	ShowWindow (i32(pagename), SW_SHOW);

	i32vfill (e.hwnd,
		i32("panel-t"), 90,
		i32(pagename), -1,
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

int panelb_oncmd (I32E e)
{
	HWND hbutton;
	int i;

	if (HIWORD(e.wp)==BM_LBUTTONUP) {
		hbutton = i32id(e.hwnd, LOWORD(e.wp));
		for (i = 101; i <= 104; i++) {
			HWND htmp = i32id(e.hwnd, i);
			SendMessage (htmp, BM_SETBCOLOR, RGB(229, 239, 254), 0);
		}
		switch (LOWORD(e.wp)) {
			case 101:
				pagename = "friendlist";
				break;
			case 102:
				pagename = "grouplist";
				break;
		}
		SendMessage (hbutton, BM_SETBCOLOR, 0xffffff, 0);
	}
	ShowWindow (i32("friendlist"), SW_HIDE);
	ShowWindow (i32("grouplist"), SW_HIDE);
	ShowWindow (i32(pagename), SW_SHOW);
	SendMessage (i32("mainform"), WM_SIZE, 0, 0);
	InvalidateRect (i32("mainform"), NULL, TRUE);
	return 0;
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

	i32box (hwnd, "n", "panel-t");
	i32set (i32("panel-t"), "bc", 0xEE9C59);
	i32setproc (i32("panel-t"), WM_SIZE, panelt_onsize);
	i32setproc (i32("panel-t"), WM_LBUTTONDOWN, panelt_lbuttondown);

	i32box (hwnd, "n", "panel-b");
	i32set (i32("panel-b"), "bc", RGB(229, 239, 254));
	i32setproc (i32("panel-b"), WM_SIZE, panelb_onsize);
	i32setproc (i32("panel-b"), WM_COMMAND, panelb_oncmd);

	/* 好友列表 */
	i32create (TEXT("chatlist"), "d|n|s|w|h|a|show", hwnd, "friendlist",
				WS_CTRL|WS_VSCROLL, 100, 100, "c", "y");

	/* 群组列表 */
	i32create (TEXT("chatlist"), "d|n|s|w|h|a|show", hwnd, "grouplist",
				WS_CTRL|WS_VSCROLL, 100, 100, "c", "y");
	SendMessage (i32("grouplist"), CM_SETVIEWMODE, 1, 0);

	hbutten = i32create(TEXT("butten"), "d|n|t|s|id|w|h|a|f|show", i32("panel-t"), "signbutton", TEXT("지 못한哈哈Эучены"),
				WS_CTRL, 101, 140, 22, "c|t", "Verdana,15", "y");
	SendMessage (hbutten, BM_SETBCOLOR, RGB(89,156,238), 0);
	SendMessage (hbutten, BM_SETFCOLOR, 0xeeeeee, 0);
	SendMessage (hbutten, BM_SETBCOLOR_HOVER, RGB(160,199,245), 0);
	SendMessage (hbutten, BM_SETBCOLOR_PUSHED, RGB(160,199,245), 0);
	SendMessage (hbutten, BM_SETRADIUS, 2, 0);
	SendMessage (hbutten, BM_SETMARGIN, 5, 0);
	SendMessage (hbutten, BM_SETTEXTCOLOR, 0x565656, 0);
	SendMessage (hbutten, BM_SETALIGN, 0, 0);
	//i32setproc (hbutten, WM_LBUTTONUP, signbutton_lbuttonup);
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
		hpanel_foot, 0,
		NULL);
	return 0;
}

int dlgpnt_onsize (I32E e)
{
	i32set(i32id(e.hwnd, 100), "a|x", "c", 3);
	i32set(i32id(e.hwnd, 200), "a|x", "c", 88);
	i32set(i32id(e.hwnd, 300), "a|x", "c", 160);

	return 0;
}

int dlg_onnote (I32E e)
{
	if (e.wp == 2100) {
		ENLINK *el = (ENLINK *)e.lp;
		if (el && el->nmhdr.code == EN_LINK) {
			HWND hrich = el->nmhdr.hwndFrom;
			if (el->msg == WM_LBUTTONDOWN) {
				TCHAR buf[320] = {0};
				int len;
				SendMessage (hrich, EM_EXSETSEL, 0, &el->chrg);
				len = SendMessage (hrich, EM_GETSELTEXT, 0, buf);
				if (len > 0)
					ShellExecute (NULL, TEXT("open"), buf, NULL, NULL, SW_SHOW);
			}
		}

	}
	return 0;
}

int footpnl_onsize (I32E e)
{
	HWND hinput = i32id(e.hwnd, 3100);
	i32vfill (e.hwnd,
		hinput, -1,
		NULL);
	i32set(hinput, "-w|-h|a|+y", 10, 18, "c", 3);
	return 0;
}

int input_onkeydown (I32E e)
{
	int st = GetKeyState(VK_SHIFT);
	if (GetKeyState(VK_SHIFT)<0 && e.wp==VK_RETURN) {
		i32set(GetParent(e.hwnd), "+h|-y", 19, 19);
		SendMessage(GetParent(GetParent(e.hwnd)), WM_SIZE, 0, 0);
		//richedit_clear(e.hwnd);
	}
	else
	if (e.wp == VK_RETURN) {
		TCHAR buf[1024];
		CHARRANGE cr = {0, -1};
		HWND chatview = i32id(i32id(GetParent(GetParent(e.hwnd)), 2000), 2100);

		richedit_gettext (e.hwnd, buf);
		richedit_clear(e.hwnd);
		richedit_setfont (chatview, FALSE, "bold|offset", TRUE, 10);
		richedit_textout (chatview, TEXT("\nCat仙:\n"));
		richedit_setfont (chatview, FALSE, "offset", 100);
		richedit_textout (chatview, buf);

		/* 滚动条加高度(估测) */
		{
			SCROLLINFO si;
			si.cbSize = sizeof(si);
			si.fMask = SIF_ALL;
			GetScrollInfo (chatview, SB_VERT, &si);
			si.fMask = SIF_RANGE|SIF_PAGE;
			si.nMin = 0;
			si.nMax += 50;
			si.nPage = max(i32clienth(chatview), 1);
			SetScrollInfo (chatview, SB_VERT, &si, TRUE);
		}

		SendMessage (chatview, WM_VSCROLL, SB_BOTTOM, 0);
		return 0;
	}


	return -1;
}

int midpnl_onsize (I32E e)
{
	HWND chatview = i32id(e.hwnd, 2100);
	i32vfill (e.hwnd,
		chatview, -1,
		NULL);
	i32set(chatview, "-w|-h|a", 6, 6, "c");
	return 0;
}

int chatview_onsize (I32E e)
{
	int clienth = i32clienth(e.hwnd);
	SCROLLINFO si;

	GetScrollInfo (e.hwnd, SB_VERT, &si);
	return -1;
}

int chatview_vscroll (I32E e)
{
	SCROLLINFO si;
	int clienth = i32clienth(e.hwnd);

	si.cbSize = sizeof(si);
	si.fMask = SIF_ALL;
	GetScrollInfo (e.hwnd, SB_VERT, &si);
	switch (LOWORD(e.wp)) {
		case SB_TOP:
			si.nPos = si.nMin;
		break;

		case SB_BOTTOM:
			si.nPos = si.nMax;
		break;

		case SB_LINEUP:
			si.nPos -= si.nMax/4;
		break;

		case SB_LINEDOWN:
			si.nPos += si.nMax/4;
		break;

		case SB_PAGEUP:
			si.nPos -= clienth;
		break;

		case SB_PAGEDOWN:
			si.nPos += clienth;
		break;

		case SB_THUMBTRACK:
			si.nPos = si.nTrackPos;
		break;
	}

	si.fMask = SIF_POS;
	SetScrollInfo (e.hwnd, SB_VERT, &si, TRUE);
	return -1;
}

void create_dlg (char *name)
{
	HWND hwnd, hpanel, hfoot, hbutten[3], hrich, hinput;
	int i;
	if (!name || i32(name)) return;

	hwnd = i32create(TEXT("form"), "n|t|s|w|h|a|x|y|bc", name,
			TEXT("对话 Dog"), WS_OVERLAPPEDWINDOW, rand()%350+100, rand()%350+300,
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
	hpanel = i32box(hwnd, "id|bc", 2000, 0xffffff);
	i32setproc (hpanel, WM_SIZE, midpnl_onsize);
	i32setproc (hpanel, WM_NOTIFY, dlg_onnote);

	/* chat view */
	hrich = new_richedit (hpanel, "id|+s|w|h|a", 2100, WS_VSCROLL, 100, 100, "c");
	richedit_autolink (hrich, TRUE);
	richedit_setfont (hrich, TRUE, "facename|size|bold|color", TEXT("Verdana"), 9, FALSE, 0x000000, TRUE);
	richedit_textout (hrich, TEXT("Go! http://cnal.com\n haha_"));
	i32setproc (hrich, WM_SIZE, chatview_onsize);
	i32setproc (hrich, WM_VSCROLL, chatview_vscroll);

	/* foot panel */
	hfoot = i32create(TEXT("box"), "s|d|id|bc|h", WS_CTRL, hwnd, 3000, 0xEE9C59, 40);
	i32setproc (hfoot, WM_SIZE, footpnl_onsize);
	/* input box */
	hinput = new_richedit(hfoot, "id|+s", 3100, WS_BORDER);
	richedit_setfont (hinput, TRUE, "facename|size|bold|color|offset", TEXT("Verdana"), 9, FALSE, 0x000000, -50);
	i32setproc (hinput, WM_KEYDOWN, input_onkeydown);

	SetFocus (hinput);

	ShowWindow (hwnd, SW_SHOW);
}

/*GROUP DLG*/
int Gmidpnl_onsize (I32E e)
{
	HWND hchatview = i32id(e.hwnd, 2100);
	HWND hlist = i32id(e.hwnd, 2200);
	HWND hsplit = i32id(e.hwnd, 2300);

 	i32hfill (e.hwnd,
		hchatview, -1,
		hsplit, 2,
		hlist, 150,
		NULL);
}

void create_groupdlg (char *name)
{
	HWND hwnd, hpanel, hfoot, hbutten[3], hrich, hinput, hlist;
	int i;
	if (!name || i32(name)) return;

	hwnd = i32create(TEXT("form"), "n|t|s|w|h|a|x|y|bc", name,
			TEXT("群对话 - 独立游戏制作"), WS_OVERLAPPEDWINDOW, 400, 400,
			"l|b", 300, 300, -1);
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
	hpanel = i32box(hwnd, "id|bc", 2000, -1);
	i32setproc (hpanel, WM_SIZE, Gmidpnl_onsize);
	i32setproc (hpanel, WM_NOTIFY, dlg_onnote);

	/* chat view */
	hrich = new_richedit (hpanel, "id|+s|w|h|a", 2100, WS_VSCROLL, 100, 100, "c");
	richedit_autolink (hrich, TRUE);
	richedit_setfont (hrich, TRUE, "facename|size|bold|color", TEXT("Verdana"), 9, FALSE, 0x000000, TRUE);
	richedit_textout (hrich, TEXT("Go! http://cnal.com\n haha_"));
	i32setproc (hrich, WM_SIZE, chatview_onsize);
	i32setproc (hrich, WM_VSCROLL, chatview_vscroll);

	/* split */
	i32box (hpanel, "id|bc", 2300, 0x000000);

	/* member list */
	hlist = i32create(TEXT("chatlist"), "s|d|id", WS_CTRL|WS_VSCROLL, hpanel, 2200);
	SendMessage (hlist, CM_SETVIEWMODE, 1, 0);

	/* foot panel */
	hfoot = i32create(TEXT("box"), "s|d|id|bc|h", WS_CTRL, hwnd, 3000, 0xEE9C59, 40);
	i32setproc (hfoot, WM_SIZE, footpnl_onsize);
	/* input box */
	hinput = new_richedit(hfoot, "id|+s", 3100, WS_BORDER);
	richedit_setfont (hinput, TRUE, "facename|size|bold|color|offset", TEXT("Verdana"), 9, FALSE, 0x000000, -50);
	i32setproc (hinput, WM_KEYDOWN, input_onkeydown);

	SetFocus (hinput);

	ShowWindow (hwnd, SW_SHOW);
}

int WINAPI WinMain (HINSTANCE hithis, HINSTANCE hiold, PSTR param, int cmd)
{
	reg_myctl ();

	create_form ();
	create_dlg ("11004");
	create_groupdlg("g001");

	i32loop();
	return 0;
}
