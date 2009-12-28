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
	I32CALLBACK f;
	struct hwndmsg *next;
} *msgtable[MSGTABLE_SIZE];



static int _init ()
{
	static BOOL inited = FALSE;
	if (inited) return 1;

	memset(nametable, 0, sizeof(nametable));
	memset(nametable, 0, sizeof(msgtable));
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
	_init ();

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
	_init ();

	hash = strhash (name) % NAMETABLE_SIZE;
	p = nametable[hash];
	if (p && p->next==NULL)
		return p->hwnd;
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

	_init ();
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


void i32set_callback (HWND hwnd, UINT message, I32CALLBACK f)
{
	struct hwndmsg **hm;
	unsigned hash;

	if (hwnd==NULL || message==0 || f==NULL) return;
	_init();

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

I32CALLBACK i32get_callback (HWND hwnd, UINT message)
{
	struct hwndmsg *p;
	unsigned hash;

	if (hwnd==NULL || message==0) return NULL;
	_init();

	hash = msghash(hwnd, message);
	p = msgtable[hash%MSGTABLE_SIZE];
	if (p && p->next==NULL)
		return p->f;
	while (p) {
		if (p->hwnd==hwnd && p->msg==message)
			return p->f;
		p = p->next;
	}
	return NULL;
}
