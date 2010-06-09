#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "tray.h"

HANDLE g_httphandle = NULL;

void httpserver_start ()
{
    TCHAR cmd[] = {0};
	STARTUPINFO si = {0};
	PROCESS_INFORMATION pi;
	memset(&pi, 0, sizeof(pi));
    memset(&si, 0, sizeof(si));
    si.cb = sizeof(si);
	si.dwFlags = STARTF_USESHOWWINDOW;
	CreateProcess (TEXT("mongoose.exe"), cmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);

	g_httphandle = pi.hProcess;
}

void httpserver_end ()
{
	TerminateProcess(g_httphandle, 0);
	g_httphandle = NULL;
}

//---------------------------

void popmenu (HWND hwnd)
{
	POINT pt;
	HMENU hmenu;

	/* 设置为前台窗口,以在单击非弹出菜单区域时使菜单自动消失! */
	SetForegroundWindow (hwnd);
	hmenu = LoadMenu (GetModuleHandle(NULL), TEXT("POPMENU"));
	hmenu = GetSubMenu (hmenu, 0);

	GetCursorPos (&pt);
	TrackPopupMenu (hmenu, TPM_RIGHTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y,
											 0, hwnd, NULL);
}

void play ()
{
	ShellExecute(NULL,TEXT("open"), TEXT("http://localhost:8079"), TEXT(""), TEXT(""), SW_SHOWMINIMIZED);
}

LRESULT CALLBACK wproc (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
		case WM_DESTROY:
			tydel(hwnd);
			PostQuitMessage (0);
		return 0;

		case ON_TRAY:
			if (wp==ID_TRAY && lp==WM_RBUTTONDOWN)
				popmenu(hwnd);
			else if (wp==ID_TRAY && lp==WM_LBUTTONUP)
				play();
		return 0;

		case WM_COMMAND:
			switch (wp) {
				case 1000:
					PostMessage (hwnd, WM_DESTROY, 0, 0);
				break;

				case 1001:
					play();
				break;

				case 1002:
                    httpserver_end ();
                    httpserver_start ();
                    MessageBox(NULL, TEXT("Server Restart successfully!"), TEXT("A of A"), MB_OK);
				break;
			}
		return 0;
	}

	return DefWindowProc (hwnd, msg, wp, lp);
}

void regclass (HINSTANCE hithis)
{
	WNDCLASS wc;

	wc.style = CS_DBLCLKS;
	wc.lpfnWndProc = wproc;
	wc.cbWndExtra = wc.cbClsExtra = 0;
	wc.hInstance = hithis;
	wc.hIcon = 0;
	wc.hCursor = LoadCursor (NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)COLOR_BACKGROUND;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = TEXT("Form");

	RegisterClass (&wc);
}

HWND createwin (HINSTANCE hithis)
{
	HWND hwnd = CreateWindow (TEXT("Form"), TEXT("The Age of Africa"), WS_OVERLAPPEDWINDOW,
           CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
           HWND_DESKTOP, NULL, hithis, NULL);
	return hwnd;
}

void msgloop ()
{
	MSG msg;

	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage (&msg);
		DispatchMessage (&msg);
	}
}

int WINAPI WinMain (HINSTANCE hThisInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR lpszArgument,
                     int nCmdShow)
{
	HWND hwnd;

	regclass (hThisInstance);
	hwnd = createwin(hThisInstance);

	tyadd(hwnd, TEXT("LOGO"), TEXT("The Age of Africa."));

	httpserver_start();
	play();
	msgloop();

	httpserver_end();
	return 0;
}
