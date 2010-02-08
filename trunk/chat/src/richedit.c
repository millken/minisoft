#include <windows.h>
#include "i32.h"
#include "ctrls.h"

#define strsame(a, b) (strcmp(a,b)==0)

HWND new_richedit (HWND dad, char *format, ...)
{
	static int dllloaded = FALSE;
	HWND  hwnd;
	DWORD style;
	va_list p;

    if (dllloaded == FALSE) {
		LoadLibrary(TEXT("Riched20.dll"));
		dllloaded = TRUE;
    }

	style = ES_MULTILINE | WS_VISIBLE | WS_CHILD | WS_TABSTOP |ES_AUTOVSCROLL | ENM_LINK |ES_WANTRETURN; /* ES_READONLY */

	hwnd= CreateWindowEx(0, RICHEDIT_CLASS, "",
		style,
		0, 0, 0, 0,
		dad, (HMENU)0, GetModuleHandle(NULL), NULL);

	va_start (p, format);
	i32vset (hwnd, format, p);
	va_end (p);

    return hwnd;
}

static char *
token (char *buf, char *s, char dot)
{
	if (!buf || !s) return NULL;

	/* isalpha比手工判断慢2倍以上 */
	while (*s && *s!=dot && *s!=' ')
		*buf++ = *s++;
	*buf = '\0';
	while (*s && (*s==dot || *s==' ')) s++;
	return s;
}

void richedit_setfont (HWND hwnd, char *format, ...)
{
	CHARFORMAT2 cf;
	char buf[32] = {0};
	va_list p;

	if (!format) return;

	memset(&cf, 0, sizeof(cf));
	cf.cbSize = sizeof(cf);
	//cf.dwMask = CFM_SIZE;

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
			cf.dwMask |= CFM_BOLD;
		}
		else
		if (strsame("color", buf)) {
			DWORD color = va_arg(p, DWORD);
			cf.dwMask |= CFM_COLOR;
			cf.crTextColor = color;
		}

		else break;


	} while (*format);
	va_end(p);

	SendMessage (hwnd, EM_SETCHARFORMAT, SCF_ALL, (WPARAM)&cf);
}
