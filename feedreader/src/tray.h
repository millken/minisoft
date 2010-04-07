#ifndef _TRAY_H
#define _TRAY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <windows.h>
#include <string.h>

#define ID_TRAY 1
#define ON_TRAY (WM_USER+1)

void tyadd (HWND hwnd, const TCHAR *iconname, const TCHAR *tip)
{
  NOTIFYICONDATA nd;

  nd.cbSize = sizeof(NOTIFYICONDATA);
  nd.hWnd = hwnd;
  nd.uID = ID_TRAY;
  nd.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
  nd.uCallbackMessage = ON_TRAY;
  nd.hIcon = LoadIcon (GetModuleHandle(NULL), iconname);
  lstrcpy (nd.szTip, tip);

  Shell_NotifyIcon (NIM_ADD, &nd);
}

void tymod (HWND hwnd, const TCHAR *iconname)
{
	NOTIFYICONDATA nd;

	memset (&nd, 0, sizeof(nd));
	nd.cbSize = sizeof(NOTIFYICONDATA);
	nd.uFlags = NIF_ICON;
	if (iconname == NULL)
		nd.hIcon = NULL;
	else
		nd.hIcon = LoadIcon (GetModuleHandle(NULL), iconname);
	nd.hWnd = hwnd;
	nd.uID = ID_TRAY;

	Shell_NotifyIcon (NIM_MODIFY, &nd);
}

void tytip (HWND hwnd, TCHAR *tip)
{
	NOTIFYICONDATA nd;

	memset (&nd, 0, sizeof(nd));
	nd.cbSize = sizeof(NOTIFYICONDATA);
	nd.uFlags = NIF_TIP;
	lstrcpy (nd.szTip, tip);
	nd.hWnd = hwnd;
	nd.uID = ID_TRAY;

	Shell_NotifyIcon (NIM_MODIFY, &nd);
}

void tydel (HWND hwnd)
{
  NOTIFYICONDATA nd;

  memset (&nd, 0, sizeof(nd));
  nd.cbSize = sizeof(nd);
  nd.uID = ID_TRAY;
  nd.hWnd = hwnd;

  Shell_NotifyIcon (NIM_DELETE, &nd);
  UpdateWindow (hwnd);
}

#ifdef __cplusplus
}
#endif

#endif
