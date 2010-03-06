#include <windows.h>
#include "i32.h"
#include "ctrls.h"

#define strsame(a, b) (strcmp(a,b)==0)

HWND create_richedit (HWND dad, char *format, ...)
{
	static int dllloaded = FALSE;
	HWND  hwnd;
	DWORD style;
	va_list p;

    if (dllloaded == FALSE) {
		LoadLibrary(TEXT("Riched20.dll"));
		dllloaded = TRUE;
    }

	style = ES_MULTILINE | WS_VISIBLE | WS_CHILD | WS_TABSTOP  |ES_WANTRETURN; /* ES_READONLY */

	hwnd= CreateWindowEx(0, RICHEDIT_CLASS, TEXT(""),
		style,
		0, 0, 0, 0,
		dad, (HMENU)0, GetModuleHandle(NULL), NULL);
	if (!hwnd) return NULL;

	if (format) {
		va_start (p, format);
		i32vset (hwnd, format, p);
		va_end (p);
	}

	/* 滚动条高度清零,原来默认是100 */
	{
		SCROLLINFO si;
		si.fMask = SIF_RANGE;
		si.nMin = 0;
		si.nMax = 0;
		SetScrollInfo (hwnd, SB_VERT, &si, TRUE);
	}

	i32setpre(hwnd);
    return hwnd;
}

static char *
token (char *buf, char *s, char dot)
{
	if (!buf || !s) return NULL;

	while (*s && *s!=dot && *s!=' ')
		*buf++ = *s++;
	*buf = '\0';
	while (*s && (*s==dot || *s==' ')) s++;
	return s;
}

void richedit_setfont (HWND hwnd, BOOL isall, char *format, ...)
{
	CHARFORMAT2 cf;
	char buf[32] = {0};
	va_list p;

	if (!format) return;

	memset(&cf, 0, sizeof(cf));
	cf.cbSize = sizeof(cf);

	va_start (p, format);
	do {
		format = token(buf, format, '|');

		if (strsame("facename", buf)) {
			TCHAR *facename = va_arg(p, TCHAR*);
			if (!facename) continue;
			cf.dwMask |= CFM_FACE;
			lstrcpy (cf.szFaceName, facename);
		}
		else
		if (strsame("size", buf)) {
			int size = va_arg(p, int);
			cf.dwMask |= CFM_SIZE;
			cf.yHeight = size*20;
		}
		else
		if (strsame("bold", buf)) {
			BOOL bold = va_arg(p, BOOL);
			if (bold) {
				cf.dwMask |= CFM_BOLD;
				cf.dwEffects |= CFE_BOLD;
			}
			else {
				cf.dwMask |= CFM_BOLD;
				cf.dwEffects &= ~CFE_BOLD;
			}
		}
		else
		if (strsame("color", buf)) {
			DWORD color = va_arg(p, DWORD);
			cf.dwMask |= CFM_COLOR;
			cf.crTextColor = color;
		}
		else
		if (strsame("offset", buf)) {
			int offset = va_arg(p, int);
			cf.dwMask |= CFM_OFFSET;
			cf.yOffset = offset;
		}

		else break;

	} while (*format);
	va_end(p);

	SendMessage (hwnd, EM_SETCHARFORMAT, isall?SCF_ALL:SCF_SELECTION, (WPARAM)&cf);
}

void richedit_textout (HWND hrich, TCHAR *text)
{
	if (!text) return;

	SendMessage(hrich, EM_REPLACESEL, (WPARAM)TRUE, (LPARAM)text);
}

/* 获得所有内容 */
void richedit_gettext (HWND hrich, TCHAR *buf)
{
	CHARRANGE cr = {0, -1};

	if (!hrich || !buf) return;

	SendMessage (hrich, EM_EXSETSEL, 0, (LPARAM)&cr);
	SendMessage (hrich, EM_GETSELTEXT, 0, (LPARAM)buf);
}

/* 清除所有内容 */
void richedit_clear (HWND hrich)
{
	SetWindowText (hrich, TEXT(""));
}

void richedit_autolink (HWND hrich, BOOL isauto)
{
	unsigned mask = SendMessage(hrich, EM_GETEVENTMASK, 0, 0);
	SendMessage(hrich, EM_SETEVENTMASK, 0, isauto?mask|ENM_LINK:mask&(~ENM_LINK));
	SendMessage (hrich, EM_AUTOURLDETECT, isauto, 0);
}
