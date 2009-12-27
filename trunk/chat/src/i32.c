#include "i32.h"

static struct hwndname {
	HWND hwnd;
	char *name;
	struct hwndname *next;
} *nametable[NAMETABLE_SIZE];


static unsigned strhash (char *s)
{
	register unsigned code = 5381;

	if (s == NULL) return 0;
	while (*s)
		code += (code << 5) + (*s++);

	return code;
}


int i32init ()
{
	static BOOL inited = FALSE;
	if (inited) return 1;

	memset(nametable, 0, sizeof(nametable));
	inited = TRUE;
	return 1;
}


void i32bind (HWND hwnd, char *name)
{
	unsigned hash;
	struct hwndname **hn;

	if (!name) return;
	i32init ();

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
	i32init ();

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

	i32init ();
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
