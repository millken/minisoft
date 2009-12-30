#include "i32.h"

#define msghash(hwnd, msg) ((unsigned)hwnd ^ (unsigned)msg)

static struct hwndname {
	HWND hwnd;
	char *name;
	struct hwndname *next;
} *nametable[NAMETABLE_SIZE];

static struct hwndmsg {
	HWND hwnd;
	UINT msg;
	I32PROC f;
	struct hwndmsg *next;
} *msgtable[MSGTABLE_SIZE];



static int init ()
{
	static BOOL inited = FALSE;
	if (inited) return 1;

	memset(nametable, 0, sizeof(nametable));
	memset(msgtable, 0, sizeof(msgtable));
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

	hash = strhash(name) % NAMETABLE_SIZE;
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


HWND i32h (char *name)
{
	unsigned hash;
	struct hwndname *p;

	if (!name) return NULL;
	init ();

	hash = strhash (name) % NAMETABLE_SIZE;
	p = nametable[hash];
	while (p) {
		if (strcmp(p->name, name) == 0)
			return p->hwnd;
		p = p->next;
	}
	return NULL;
}


HWND i32create (char *classname, char *name)
{
	HWND hwnd;

	init ();
	hwnd = CreateWindow (
           classname,         	/* Classname */
           NULL,       			/* Title Text */
           WS_OVERLAPPEDWINDOW,					/* default window */
           CW_USEDEFAULT,		/* Windows decides the position */
           CW_USEDEFAULT,		/* where the window ends up on the screen */
           CW_USEDEFAULT,		/* The programs width */
           CW_USEDEFAULT,		/* and height in pixels */
           HWND_DESKTOP,        /* The window is a child-window to desktop */
           NULL,                /* No menu */
           GetModuleHandle(NULL),       /* Program Instance handler */
           NULL                 /* No Window Creation data */
	);
	i32bind (hwnd, name);

	return hwnd;
}


static void set_proc (HWND hwnd, UINT message, I32PROC f)
{
	struct hwndmsg **hm;
	unsigned hash;

	hash = msghash(hwnd, message);
	hm = &msgtable[hash%MSGTABLE_SIZE];
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
	p = msgtable[hash%MSGTABLE_SIZE];
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

	thisproc = i32getproc (hwnd, message);
	if (thisproc)
		return thisproc (hwnd, wp, lp);

	oldproc = (WNDPROC)i32getproc (hwnd, 0);
	return oldproc ? oldproc(hwnd, message, wp, lp) : 0;
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
	for (i = 0; i < NAMETABLE_SIZE; i++) {
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
	for (i = 0; i < MSGTABLE_SIZE; i++) {
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
