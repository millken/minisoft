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
	int an; /* count of attributes */
	int linemaxh; /* max height of this line */
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


/* 排版&渲染,
 * 参数:
 * todraw: false为排版, true为渲染,
 * r->left: 左边界x坐标,
 * r->top: 上边界y坐标,
 * r->right: 宽度,
 * r->bottom: 首行起点x,
 * lh: 每个渲染行的第一个tag, 外部调用传NULL
 *
 * 返回: 元素底部y坐标
 */
int layout_render (bool todraw, HDC hdc, glnode* dad, RECT* wrap, Tag* lh)
{
	static Tag* linehead = NULL;

	glnode* node;
	Tag* tag;
	wchar_t* ws;
	int len;
	int left, top, width, height = 0;
	int dadh = 0;

	if (!dad) return 0;

	top = wrap->top;
	left = wrap->bottom; //起点
	width = wrap->right;

	if (lh == NULL)
		linehead = dad->child;
	if (linehead == NULL)
		return top;

	glforeach (dad, node) {
		tag = (Tag*)node->data;

		if (tag->type == TEXT) {
			//printf ("%d\n", tag->len);
			int rowlen = 0;
			ws = tag->wname;
			len = tag->namelen;

			if (len <= 0) continue;
			// walk line by line
			while (len > 0) {
				SIZE size;
				bool nextline;

				// 超出边界
				if (left > wrap->right || left < 0)
					left = wrap->left;

				width = wrap->right - left;
				GetTextExtentExPointW(hdc, ws, len, width, &rowlen, NULL, &size);
				height = size.cy+2;
				nextline = left + size.cx > wrap->right;

				// from the line start
				if (left == wrap->left) {
					linehead = tag;
					printf ("text: %s, linemaxh: %d\n", tag->name, linehead->linemaxh);
					top += (left==wrap->left&&nextline) ? height : linehead->linemaxh;
				}

				// layout, caculate line max height
				if (!todraw) {
					if (linehead->linemaxh < height)
						linehead->linemaxh = height;
					if (left == wrap->left) {
						linehead = tag;
						linehead->linemaxh = height;
					}
				}

				if (todraw) {
					//printf ("linemaxh: %d\n", linehead->linemaxh);
					//printf("left: %d, top: %d\n", left, top);
					SetTextAlign(hdc, TA_LEFT|TA_BOTTOM|TA_NOUPDATECP);
					ExtTextOutW(hdc, left, top, ETO_OPAQUE, NULL, ws, rowlen, NULL);
				}

				// next text-segment
				ws += rowlen;
				len -= rowlen;

				// get x
				if (nextline)
					left = wrap->left;
				else
					left += size.cx;
			}

		}
		else if (tag->type == ELEMENT) {
			// img [原子元素]
			if (strcmp(tag->name, "img") == 0) {
				int iw = 86, ih = 46;
				bool nextline = (left > wrap->left && left+iw > wrap->right);

				if (left > wrap->left && left+iw > wrap->right) {
					// next line
					left = wrap->left;
				}

				if (left == wrap->left) {
					linehead = tag;
					printf ("img linemax: %d\n", linehead->linemaxh);
					top += linehead->linemaxh;
				}

				// layout, caculate line max height
				if (!todraw) {
					if (left == wrap->left) {
						linehead = tag;
						linehead->linemaxh = ih + 1; //margin-bottom 1
					}
					else if (linehead->linemaxh < ih)
						linehead->linemaxh = ih + 1;
				}

				RECT r = {left+1, top-ih, left+iw+1, top};
				if (todraw) {
					//printf ("r->top: %d\n", r.top);
					i32fillrect(hdc, &r, 0x00ccff);
				}
				left = r.right;
			}
			// span [行容器], 本身无宽,由子元素决定
			else if (strcmp(tag->name, "span") == 0) {
				RECT r = {wrap->left, top, wrap->right, left};
				top = layout_render (todraw, hdc, node, &r, linehead);
				left = r.bottom;
			}
			// div [块容器], 自己占一行, 宽度100%
			else if (strcmp(tag->name, "div") == 0) {
				RECT r = {wrap->left, top, wrap->right, wrap->left};
				// new line
				linehead = tag;
				left = wrap->left;
				top = layout_render (todraw, hdc, node, &r, linehead);
				// new line
				linehead = tag;
				left = wrap->left;
			}
		}

	}
	wrap->bottom = left;
	return top;
}

LRESULT CALLBACK winproc (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	static glnode* root = NULL;

	switch (msg) {
		case M_SETROOT:
			root = (glnode*)wp;
		return 0;

		case WM_SIZE: {
			HDC hdc = GetDC(hwnd);
			RECT rect;
			GetClientRect (hwnd, &rect);
			rect.right -= rect.left;
			rect.bottom = rect.left;
			layout_render (false, hdc, root, &rect, NULL);
			ReleaseDC(hwnd, hdc);
			return 0;
		}

		case WM_PAINT: {
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);
			RECT rect;
			GetClientRect (hwnd, &rect);
			rect.right -= rect.left;
			rect.bottom = rect.left;
			layout_render (true, hdc, root, &rect, NULL);
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
