#include "i32.h"
#include <ctype.h>

#define msghash(hwnd, msg) ((unsigned)hwnd ^ (unsigned)msg)
#define STRSAME(a, b) (strcmp((a),(b))==0)

static struct hwndname {
	HWND hwnd;
	char *name;
	struct hwndname *next;
} *nametable[I32NAMETABLE_SIZE];

static struct hwndmsg {
	HWND hwnd;
	UINT msg;
	I32PROC f;
	struct hwndmsg *next;
} *msgtable[I32MSGTABLE_SIZE];



/*****************************
 * 提供三个容器控件
 *****************************/

/* 普通容器,可作为主窗口 */
static LRESULT CALLBACK
box_proc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
		case WM_DESTROY: {
			HWND dad = GetParent (hwnd);
			if (!dad) /* 判断是主窗口才敢退出程序 */
				PostQuitMessage (0);
		}
		break;
	}
    return DefWindowProc (hwnd, message, wParam, lParam);
}

static int reg_box ()
{
	WNDCLASSEX wincl;

    /* The Window structure */
    wincl.hInstance = GetModuleHandle(0);
    wincl.lpszClassName = "box";
    wincl.lpfnWndProc = box_proc;      /* This function is called by windows */
    wincl.style = CS_DBLCLKS | CS_VREDRAW | CS_HREDRAW;                 /* Catch double-clicks */
    wincl.cbSize = sizeof (WNDCLASSEX);

    /* Use default icon and mouse-pointer */
    wincl.hIcon = LoadIcon (NULL, IDI_APPLICATION);
    wincl.hIconSm = LoadIcon (NULL, IDI_APPLICATION);
    wincl.hCursor = LoadCursor (NULL, IDC_ARROW);
    wincl.lpszMenuName = NULL;                 /* No menu */
    wincl.cbClsExtra = 0;                      /* No extra bytes after the window class */
    wincl.cbWndExtra = 0;                      /* structure or the window instance */
    /* Use Windows's default colour as the background of the window */
    wincl.hbrBackground = (HBRUSH) COLOR_BACKGROUND;

    /* Register the window class, and if it fails quit the program */
    return !!RegisterClassEx (&wincl);
}


static int init ()
{
	static BOOL inited = FALSE;
	if (inited) return 1;

	memset(nametable, 0, sizeof(nametable));
	memset(msgtable, 0, sizeof(msgtable));
	reg_box ();
	inited = TRUE;
	return 1;
}


static unsigned strhash (char *s)
{
	register unsigned code = 5381;

	if (s == NULL) return 0;
	while (*s)
		code += (code << 5) + (*s++);

	return code;
}


void i32bind (HWND hwnd, char *name)
{
	unsigned hash;
	struct hwndname **hn;

	if (!name) return;
	init ();

	hash = strhash(name) % I32NAMETABLE_SIZE;
	hn = &nametable[hash];
	while (*hn) {
		struct hwndname *p = *hn;
		if (strcmp(p->name, name) == 0) {
			p->hwnd = hwnd;
			return;
		}
		hn = &p->next;
	}
	*hn = (struct hwndname *)i32malloc (sizeof(struct hwndname));
	(*hn)->hwnd = hwnd;
	(*hn)->name = name;
	(*hn)->next = NULL;
}


HWND i32 (char *name)
{
	unsigned hash;
	struct hwndname *p;

	if (!name) return NULL;
	init ();

	hash = strhash (name) % I32NAMETABLE_SIZE;
	p = nametable[hash];
	while (p) {
		if (strcmp(p->name, name) == 0)
			return p->hwnd;
		p = p->next;
	}
	return NULL;
}


static void set_proc (HWND hwnd, UINT message, I32PROC f)
{
	struct hwndmsg **hm;
	unsigned hash;

	hash = msghash(hwnd, message);
	hm = &msgtable[hash%I32MSGTABLE_SIZE];
	while (*hm) {
		struct hwndmsg *p = *hm;
		if (p->hwnd==hwnd && p->msg==message) {
			p->f = f;
			return;
		}
		hm = &p->next;
	}
	*hm = (struct hwndmsg *)i32malloc(sizeof(struct hwndmsg));
	(*hm)->hwnd = hwnd;
	(*hm)->msg = message;
	(*hm)->f = f;
	(*hm)->next = NULL;
}


I32PROC i32getproc (HWND hwnd, UINT message)
{
	struct hwndmsg *p;
	unsigned hash;

	if (hwnd==NULL) return NULL;
	init();

	hash = msghash(hwnd, message);
	p = msgtable[hash%I32MSGTABLE_SIZE];
	while (p) {
		if (p->hwnd==hwnd && p->msg==message)
			return p->f;
		p = p->next;
	}
	return NULL;
}


static LRESULT CALLBACK
i32defproc (HWND hwnd, UINT message, WPARAM wp, LPARAM lp)
{
	I32PROC thisproc;
	WNDPROC oldproc;
	int r;

	oldproc = (WNDPROC)i32getproc (hwnd, 0);
	r = CallWindowProc (oldproc, hwnd, message, wp, lp);

	thisproc = i32getproc (hwnd, message);
	if (thisproc) {
		I32EVENT e = {hwnd, wp, lp};
		return thisproc (e);
	}
	return r;
}

static void bind_defproc (HWND hwnd)
{
	I32PROC oldproc;

	oldproc = i32getproc (hwnd, 0);
	if (oldproc) {return;}

	oldproc = (I32PROC)GetWindowLong(hwnd, GWL_WNDPROC);
	set_proc (hwnd, 0, oldproc);
	SetWindowLong (hwnd, GWL_WNDPROC, (LONG)i32defproc);
}

void i32setproc (HWND hwnd, UINT message, I32PROC f)
{
	if (hwnd==NULL || message==0 || f==NULL) return;
	init();
	bind_defproc (hwnd);

	set_proc (hwnd, message, f);
}


void i32debug ()
{
	int i;

	printf ("name table:\n");
	for (i = 0; i < I32NAMETABLE_SIZE; i++) {
		printf ("%u", nametable[i]!=0);
		if (nametable[i]) {
			struct hwndname *p = nametable[i];
			printf ("(");
			while (p) {
				printf ("[%u:%s],", p->hwnd, p->name);
				p = p->next;
			}
			printf (")");
		}
		else printf (" ");
		puts("\r");
	}

	printf ("\nmsg table:\n");
	for (i = 0; i < I32MSGTABLE_SIZE; i++) {
		printf ("%u", msgtable[i]!=0);
		if (msgtable[i]) {
			struct hwndmsg *p = msgtable[i];
			printf ("(");
			while (p) {
				printf ("[%u:%u],", p->hwnd, p->msg);
				p = p->next;
			}
			printf (")");
		}
		else printf (" ");
		puts("\r");
	}
}

static char *
token (char *buf, char *s)
{
	assert (buf && s);

	/* isalpha比单独判断符号慢2倍以上 */
	while (*s && *s!=I32SEP && *s!=' ')
		*buf++ = *s++;
	*buf = '\0';
	while (*s && (*s==I32SEP || *s==' ')) s++;
	return s;
}


void i32vset (HWND hwnd, char *format, va_list p)
{
	char a[16];

	if (format == NULL)
		return;

	do {
		format = token(a, format);

		if (STRSAME("name", a)) {
			char *name = va_arg(p, char*);
			i32bind(hwnd, name);
		}
		else
		if (STRSAME("style", a)) {
			LONG style = va_arg(p, LONG);
			SetWindowLong(hwnd, GWL_STYLE, style);
		}

		else
		if (STRSAME("+style", a)) {
			LONG style = va_arg(p, LONG);
			style |= GetWindowLong(hwnd, GWL_STYLE);
			SetWindowLong(hwnd, GWL_STYLE, style);
		}
		else
		if (STRSAME("-style", a)) {
			LONG style = va_arg(p, LONG);
			style = GetWindowLong(hwnd, GWL_STYLE) & ~style;
			SetWindowLong(hwnd, GWL_STYLE, style);
		}
		else
		if (STRSAME("title", a)) {
			char *title = va_arg(p, char*);
			SetWindowText (hwnd, title);
		}
		else
		if (STRSAME("size", a)) {
			POINT size = va_arg(p, POINT);
			RECT r;
			GetWindowRect (hwnd, &r);
			MoveWindow (hwnd, r.left, r.top, size.x, size.y, TRUE);
		}
		else
		if (STRSAME("pos", a)) {
			POINT pos = va_arg(p, POINT);
			RECT r;
			GetWindowRect (hwnd, &r);
			MoveWindow (hwnd, pos.x, pos.y, r.right-r.left, r.bottom-r.top, TRUE);
		}
		else
		if (STRSAME("rect", a)) {
			RECT r = va_arg(p, RECT);
			MoveWindow (hwnd, r.left, r.top, r.right, r.bottom, TRUE);
		}
		else
		if (STRSAME("x", a) || STRSAME("left", a)) {
			int x = va_arg(p, int);
			RECT r;
			HWND dad = GetParent (hwnd);
			GetWindowRect (hwnd, &r);
			r.right -= r.left;
			r.bottom -= r.top;
			ScreenToClient (dad, &r);
			MoveWindow (hwnd, x, r.top, r.right, r.bottom, TRUE);
		}
		else
		if (STRSAME("+x", a)) {
			int dx = va_arg(p, int);
			RECT r;
			HWND dad = GetParent (hwnd);
			GetWindowRect (hwnd, &r);
			r.right -= r.left;
			r.bottom -= r.top;
			ScreenToClient (dad, &r);
			MoveWindow (hwnd, r.left+dx, r.top, r.right, r.bottom, TRUE);
		}
		else
		if (STRSAME("-x", a)) {
			int dx = va_arg(p, int);
			RECT r;
			HWND dad = GetParent (hwnd);
			GetWindowRect (hwnd, &r);
			r.right -= r.left;
			r.bottom -= r.top;
			ScreenToClient (dad, &r);
			MoveWindow (hwnd, r.left-dx, r.top, r.right, r.bottom, TRUE);
		}
		else
		if (STRSAME("y", a) || STRSAME("top", a)) {
			int y = va_arg(p, int);
			RECT r;
			HWND dad = GetParent (hwnd);
			GetWindowRect (hwnd, &r);
			r.right -= r.left;
			r.bottom -= r.top;
			ScreenToClient (dad, &r);
			MoveWindow (hwnd, r.left, y, r.right, r.bottom, TRUE);
		}
		else
		if (STRSAME("+y", a)) {
			int dy = va_arg(p, int);
			RECT r;
			HWND dad = GetParent (hwnd);
			GetWindowRect (hwnd, &r);
			r.right -= r.left;
			r.bottom -= r.top;
			ScreenToClient (dad, &r);
			MoveWindow (hwnd, r.left, r.top+dy, r.right, r.bottom, TRUE);
		}
		else
		if (STRSAME("-y", a)) {
			int dy = va_arg(p, int);
			RECT r;
			HWND dad = GetParent (hwnd);
			GetWindowRect (hwnd, &r);
			r.right -= r.left;
			r.bottom -= r.top;
			ScreenToClient (dad, &r);
			MoveWindow (hwnd, r.left, r.top-dy, r.right, r.bottom, TRUE);
		}
		else
		if (STRSAME("w", a) || STRSAME("width", a)) {
			int w = va_arg(p, int);
			RECT r;
			HWND dad = GetParent (hwnd);
			GetWindowRect (hwnd, &r);
			r.right -= r.left;
			r.bottom -= r.top;
			ScreenToClient (dad, &r);
			MoveWindow (hwnd, r.left, r.top, w, r.bottom, TRUE);
		}
		else
		if (STRSAME("h", a) || STRSAME("height", a)) {
			int h = va_arg(p, int);
			RECT r;
			HWND dad = GetParent (hwnd);
			GetWindowRect (hwnd, &r);
			r.right -= r.left;
			r.bottom -= r.top;
			ScreenToClient (dad, &r);
			MoveWindow (hwnd, r.left, r.top, r.right, h, TRUE);
		}
		else
		if (STRSAME("align", a)) {
			char *v = va_arg(p, char*);
			HWND dad = GetParent(hwnd);
			RECT dr, r;
			int w, h, dw, dh;
			char pos[16];

			if (dad) {
				GetClientRect (dad, &dr);
				dw = dr.right - dr.left;
				dh = dr.bottom - dr.top;
			} else {
				dw = GetSystemMetrics(SM_CXSCREEN);
				dh = GetSystemMetrics(SM_CYSCREEN);
			}
			GetWindowRect (hwnd, &r);
			w = r.right - r.left;
			h = r.bottom - r.top;
			ScreenToClient (dad, &r);
			do {
				v = token(pos, v);
				if (STRSAME("center", pos)) {
					r.left = (dw-w)/2;
					r.top = (dh-h)/2;
				}
				else if (STRSAME("left", pos))
					r.left = 0;
				else if (STRSAME("right", pos))
					r.left = dw - w;
				else if (STRSAME("top", pos))
					r.top = 0;
				else if (STRSAME("bottom", pos))
					r.top = dh - h;
			} while (*v != '\0');
			MoveWindow (hwnd, r.left, r.top, w, h, TRUE);
		} /*endif align*/

		else
		if (STRSAME("dad", a)) {
			HWND dad = va_arg(p, HWND);
			SetParent (hwnd, dad);
		}

	} while (*format != '\0');
}

void i32set (HWND hwnd, char *format, ...)
{
	va_list p;

	va_start (p, format);
	i32vset (hwnd, format, p);
	va_end (p);
}


HWND i32create (char *classname, char *format, ...)
{
	HWND hwnd;
	va_list p;

	init ();
	hwnd = CreateWindow (
           classname,         	/* Classname */
           NULL,       			/* Title Text */
           WS_OVERLAPPEDWINDOW,					/* default window */
           0,		/* Windows decides the position */
           0,		/* where the window ends up on the screen */
           CW_USEDEFAULT,		/* The programs width */
           CW_USEDEFAULT,		/* and height in pixels */
           HWND_DESKTOP,        /* The window is a child-window to desktop */
           NULL,                /* No menu */
           GetModuleHandle(NULL),       /* Program Instance handler */
           NULL                 /* No Window Creation data */
	);

	va_start (p, format);
	i32vset(hwnd, format, p);
	va_end(p);

	return hwnd;
}


int i32oldproc (UINT message, I32EVENT e)
{
	WNDPROC proc = (WNDPROC)i32getproc (e.hwnd, 0);
	return CallWindowProc(proc, e.hwnd, message, e.wp, e.lp);
}

int i32msgloop ()
{
	MSG messages;

    /* Run the message loop. It will run until GetMessage() returns 0 */
    while (GetMessage (&messages, NULL, 0, 0))
    {
        /* Translate virtual-key messages into character messages */
        TranslateMessage(&messages);
        /* Send message to WindowProcedure */
        DispatchMessage(&messages);
    }
    return messages.wParam;
}
