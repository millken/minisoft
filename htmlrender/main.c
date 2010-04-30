#include <windows.h>
#include "i32.h"
#include "ds.h"
#include <ctype.h>

enum {
	M_SETROOT = WM_USER + 1001
};

/*

function lex (dad) {
	text, tag, text, tag ...

	loop read brothers {
		dad.child[i] = node = '<' + tagname + attrs + '>';
		lex node;
		closetag = '</' + tagname + '>';
		i++;
	} util closetag.name != dad.tag or eof;
}

*/

void erase_noprint (char* text, int len)
{
	int i;

	for (i = 0; i < len; i++)
		if (text[i]=='\r' || text[i]=='\n')
			text[i] = ' ';
}

typedef enum {
	TEXT,
	ELEMENT
} TagType;

typedef struct {
	char *k, *v;
} Attr;

typedef struct {
	TagType type;
	char* name;
	wchar_t* wname;
	size_t namelen; /* char count of wname */
	Attr* attrs;
	int an;
	int y; /* line bottom */
} Tag;

Tag* newtag (TagType type)
{
	Tag* tag = (Tag*)salloc(sizeof(Tag));
	tag->type = type;
	return tag;
}

void deltag (Tag* tag)
{
	int i;

	if (tag) {
		sfree(tag->name);
		sfree(tag->wname);
		for (i = 0; i < tag->an; i++) {
			sfree (tag->attrs[i].k);
			sfree (tag->attrs[i].v);
		}
		sfree(tag->attrs);
		free(tag);
	}
}

void _parseElem (Tag* tag, char* text)
{
	char *s;
	char *k, *v;
	int klen, vlen;
	int n;
	char brace;
	Attr* a;

	for (s = text; isspace(*s); s++);
	if (*s == '\0') return;
	text = s;

	// parse tagname
	for (s = text; *s && !isspace(*s); s++);
	tag->name = sdumpn(text, s-text);
	if (*s == '\0') return;
	text = s;

	// get how many attributes
	for (n = 0, s = text; *s; s++)
		if (*s == '=')
			n++;
	if (n == 0) return;

	tag->attrs = (Attr*)salloc(sizeof(Attr)*n);
	tag->an = 0;

	// parse attributes
	while (true) {
		for (s = text; isspace(*s); s++);
		if (*s == '\0') return;
		text = s;

		// key
		for (k = s = text; *s && *s!='='; s++);
		if (*s == '\0') return;
		klen = s - k;

		// value
		s++; //跳过等号
		brace = *s;
		if (brace=='"' || brace=='\'') {
			s++; //跳过第一个引号
			for (v = s; *s && *s!=brace; s++);
			vlen = s - v;
			if (*s) s++;
		}
		else {
			for (v = s; *s && !isspace(*s); s++);
			vlen = s - v;
		}

		if (klen > 0) {
			a = &tag->attrs[tag->an++];
			a->k = sdumpn(k, klen);
			a->v = sdumpn(v, vlen);
		}

		if (*s == '\0') return;
		text = s;
	}
}

void parseElem (Tag* tag, char* text, size_t len)
{
	char endchar = text[len];
	text[len] = '\0';
	_parseElem (tag, text);
	text[len] = endchar;
}

char* lex (char* text, glnode* dad)
{
	glnode* node;
	Tag* tag;
	char* s, *dadname;

	if (!dad || !dad->data || !text)
		return NULL;
	if (*text=='\0')
		return text;

	tag = (Tag*)dad->data;
	dadname = tag->name ? tag->name : "";

	// loop brothers
	while (true) {
		// parse text, go to '<'
		for (s = text; isspace(*s); s++);
		text = s;
		for (s = text; *s && *s!='<'; s++);
		if (s > text) {
			tag = newtag (TEXT);
			erase_noprint (text, s - text);
			tag->name = trimn (text, s - text);
			tag->wname = utf2unicode(tag->name);
			tag->namelen = lstrlen(tag->wname);
			gljoin(dad, glnew(tag), -1);
		}

		if (*s == '\0') return text;

		// parse tag, go to '>'
		text = s + 1;
		for (s = text; *s && *s!='>'; s++);

		// if tag close, 只匹配父元素
		if (*s=='>' && text[0]=='/') {
			text += 1;
			if (strncmp(text, dadname, s-text) == 0)
				return s+1;
			// 失败,跳过继续解析文本
			text = s + 1;
			continue;
		}

		// if tag open
		if (*s=='>' &&  s > text) {
			int closed = s[-1]=='/';

			tag = newtag(ELEMENT);
			// close self
			parseElem(tag, text, s-text-closed);
			node = glnew(tag);
			gljoin (dad, node, -1);

			text = closed ? s+1 : lex(s+1, node);

			if (!text) return NULL;
		}

		if (*text == '\0') return text;
	}

	return text;
}

void print_gl (glnode* node, int indent)
{
	glnode* p;
	Tag* tag;
	int i, j;

	if (!node) return;

	glforeach (node, p) {
		if (p && p->data) {
			tag = (Tag*)p->data;
			for (i = 0; i < indent; i++)
				printf (" ");
			if (tag->type == ELEMENT)
				printf ("[%s]", tag->name);
			else
				printf ("%s", tag->name);
			if (tag->an > 0) {
				printf (" ");
				for (j = 0; j < tag->an; j++)
					printf ("%s='%s' ", tag->attrs[j].k, tag->attrs[j].v);
			}
			printf ("\n");

			print_gl(p, indent+1);
		}
	}
}

void tagfreefunc (void* data)
{
	deltag((Tag*)data);
}

/* 确定本行高度 */
int rowheight (HDC hdc, glnode* firstnode, int left, int width)
{
	glnode* p;
	Tag* tag;
	int maxheight = 0;

	if (!firstnode) return 0;

	for (p = firstnode; p; p=p->next) {
		tag = (Tag*)p->data;
		if (tag->type == TEXT) {
			maxheight = max(maxheight, 12);
		}
		else
			maxheight = max(maxheight, 20);
	}

	return maxheight;
}

void render_gl (HDC hdc, glnode* dad, RECT* r)
{
	glnode* node;
	Tag* tag;
	wchar_t* ws;
	int len;
	int left, top, width, h;

	SetTextAlign(hdc, TA_LEFT|TA_BOTTOM|TA_NOUPDATECP);

	top = r->top;
	left = r->left;
	width = r->right - left;

	h = rowheight (hdc, dad->child, left, width);
	printf ("rowh: %d, cn:%d\n", h, dad->cn);
	top += h;

	glforeach (dad, node) {
		tag = (Tag*)node->data;
		if (tag->type == TEXT) {
			//printf ("%d\n", tag->len);
			int rowlen = 0;
			ws = tag->wname;
			len = tag->namelen;

			if (len <= 0) continue;

			while (len > 0) {
				SIZE size;
				width = r->right - left;
				GetTextExtentExPointW(hdc, ws, len, width, &rowlen, NULL, &size);
				ExtTextOutW(hdc, left, top, ETO_OPAQUE, NULL, ws, rowlen, NULL);

				// next segent
				ws += rowlen;
				len -= rowlen;

				// next line
				if (left + size.cx > r->right) {
					left = r->left;
					top += size.cy+2;
				}
				else
					left += size.cx;
			}

		}
		else if (tag->type == ELEMENT) {
			if (strcmp(tag->name, "img") == 0) {
				int iw = 16, ih = 16;
				if (left+iw > r->right) {
					// next line
					top += ih;
					left = r->left;
				}
				RECT r = {left, top-ih, left+iw, top};
				i32fillrect(hdc, &r, 0x00ccff);
				left = r.right;
			}
		}

	}

}

LRESULT CALLBACK winproc (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	static glnode* root = NULL;

	switch (msg) {
		case M_SETROOT:
			root = (glnode*)wp;
		return 0;

		case WM_PAINT: {
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);
			RECT rect;
			GetClientRect (hwnd, &rect);
			render_gl (hdc, root, &rect);
			EndPaint(hwnd, &ps);
			return 0;
		}
	}

	return DefWindowProc(hwnd, msg, wp, lp);
}

void regist ()
{
	WNDCLASSEX wincl;

    /* The Window structure */
    wincl.hInstance = GetModuleHandle(0);
    wincl.lpszClassName = TEXT("tinyhtml");
    wincl.lpfnWndProc = winproc;      /* This function is called by windows */
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
    wincl.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);

    /* Register the window class, and if it fails quit the program */
   RegisterClassEx (&wincl);
}

int on_formclose (I32E e) {
	PostQuitMessage(0);
	return 0;
}

int on_formsize (I32E e) {
	HWND hview = i32("htmlview");
	i32vfill (e.hwnd,
		hview, -1,
		NULL);
	return 0;
}

int WINAPI WinMain (HINSTANCE hithis, HINSTANCE hipre, PSTR param, int icmd)
{
	char* buf, *s;
	glnode* root;

	buf = fdump ("index.htm");
	root = glnew(newtag(ELEMENT));
	s = lex (buf, root);

	print_gl (root, 0);

	///window
	{
	HWND hwnd, hview;
	regist();
	hwnd = i32box (NULL, "s|t|w|h|a|-y|bc", WS_OVERLAPPEDWINDOW, TEXT("tinyhtml"), 200, 200, "c", 100, -1);
	i32setproc (hwnd, WM_DESTROY, on_formclose);
	i32setproc (hwnd, WM_SIZE, on_formsize);
	hview = i32create (TEXT("tinyhtml"), "n|d|s|w|h|a", "htmlview", hwnd, WS_CTRL, 100, 100, "c");
	i32send (hview, M_SETROOT, root, 0);

	ShowWindow (hwnd, SW_SHOW);
	i32loop();
	}
	///window


	gldel(root, tagfreefunc);
	sfree(buf);

	return 0;
}
