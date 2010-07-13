#include <windows.h>
#include "i32.h"
#include "ds.h"
#include <ctype.h>

enum {
	M_SETROOT = WM_USER + 1001
};

typedef enum {
	DISPLAY_BLOCK = 0, //块对象
	DISPLAY_INLINE, //内联对象
	DISPLAY_NONE, //隐藏对象

	POSITION_STATIC = 0, //默认值。无特殊定位，对象遵循HTML定位规则
	POSITION_ABSOLUTE, //绝对定位, 使用 left ， right ， top ， bottom 等属性相对于其最接近的
					   //一个最有定位设置的父对象进行绝对定位。如果不存在这样的父对象，则依据 body 对象。
	POSITION_FIXED, //屏幕位置
	POSITION_RELATIVE, //相对定位, 在正常文档流中偏移位置

	FLOAT_NONE = 0,
	FLOAT_LEFT,
	FLOAT_RIGHT,

	OVERFLOW_VISIBLE = 0, //默认值。不剪切内容也不添加滚动条。
	OVERFLOW_AUTO, //在必需时对象内容才会被裁切或显示滚动条
	OVERFLOW_HIDDEN, //不显示超过对象尺寸的内容
	OVERFLOW_SCROLL, //总是显示滚动条

	FONTSTYLE_NORMAL = 0,
	FONTSTYLE_ITALIC, //斜体
	FONTSTYLE_OBLIQUE, //倾斜的字体

	FONTWEIGHT_NORMAL = 400,
	FONTWEIGHT_BOLD = 700,
	FONTWEIGHT_BOLDER = 800,
	FONTWEIGHT_LIGHTER = 100,

	TEXTALIGN_LEFT = 0, //默认值。左对齐
	TEXTALIGN_RIGHT,
	TEXTALIGN_CENTER, //居中对齐
	TEXTALIGN_JUSTIFY, //两端对齐

	TEXTDEC_NONE = 0, //默认值。无装饰
	TEXTDEC_UNDERLINE, //下划线
	TEXTDEC_BLINK, //闪烁
	TEXTDEC_OVERLINE, //上划线
	TEXTDEC_THROUGH, //贯穿线

	BORDERSTYLE_NONE = 0,
	BORDERSTYLE_HIDDEN,
	BORDERSTYLE_DOTTED,
	BORDERSTYLE_DASHED,
	BORDERSTYLE_SOLID,
	BORDERSTYLE_DOUBLE,

	VIS_INHERIT = 0,
	VIS_VISIBLE,
	VIS_COLLAPSE,
	VIS_HIDDEN,

} CSSCONST;

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
	uint display; //none, block, inline
	uint position;
	uint _float;
	uint visibility;
	uint overflow_x;
	uint overflow_y;

	int left;
	int top;
	int right;
	int bottom;
	int height;
	int min_height;
	int max_height;
	int width;
	int min_width;
	int max_width;

	int margin_left;
	int margin_top;
	int margin_right;
	int margin_bottom;
	int padding_left;
	int padding_top;
	int padding_right;
	int padding_bottom;

	uint color;
	uint background_color;

	uint border_top_color;
	int border_top_width;
	uint border_top_style;
	uint border_right_color;
	int border_right_width;
	uint border_right_style;
	uint border_left_color;
	int border_left_width;
	uint border_left_style;
	uint border_bottom_color;
	int border_bottom_width;
	uint border_bottom_style;

	char* font_family; //const string
	uint font_size;
	uint font_style;
	int font_weight;

	uint text_align;
	uint text_decoration;
	int line_height;

} CSS;

typedef struct {
	char* _class;
	char* id;
	bool checked;
	bool disabled;
	char* src;
	char* href;
	char* title;
	char* value;
} ATTR;

typedef struct {
	TagType type;
	char* name; /* utf-8 name */
	wchar_t* wname; /* unicode name */
	size_t namelen; /* char count of tagname */
	Attr* attrs;
	int an; /* count of attributes */
	CSS css;
	ATTR attr;
	int linemaxh; /* for layout, max height of this line */
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

void setstyle (Tag* tag, char* k, int klen, char* v, int vlen)
{
	char* val = sdumpn(v, vlen);

	if (strncmp(k, "display", klen) == 0) {
		if (strncmp(v, "none", vlen) == 0)
			tag->css.display = DISPLAY_NONE;
		else if (strncmp(v, "block", vlen) == 0)
			tag->css.display = DISPLAY_BLOCK;
		else if (strncmp(v, "inline", vlen) == 0)
			tag->css.display = DISPLAY_INLINE;
	}
	else if (strncmp(k, "position", klen) == 0) {
		if (strncmp(v, "static", vlen) == 0)
			tag->css.position = POSITION_STATIC;
		else if (strncmp(v, "absolute", vlen) == 0)
			tag->css.position = POSITION_ABSOLUTE;
		else if (strncmp(v, "fixed", vlen) == 0)
			tag->css.position = POSITION_FIXED;
		else if (strncmp(v, "relative", vlen) == 0)
			tag->css.position = POSITION_RELATIVE;
	}
	else if (strncmp(k, "float", klen) == 0) {
		if (strncmp(v, "none", vlen) == 0)
			tag->css._float = FLOAT_NONE;
		else if (strncmp(v, "left", vlen) == 0)
			tag->css._float = FLOAT_LEFT;
		else if (strncmp(v, "right", vlen) == 0)
			tag->css._float = FLOAT_RIGHT;
	}
	else if (strncmp(k, "visibility", klen) == 0) {
		if (strncmp(v, "inherit", vlen) == 0)
			tag->css.visibility = VIS_INHERIT;
		else if (strncmp(v, "visible", vlen) == 0)
			tag->css.visibility = VIS_VISIBLE;
		else if (strncmp(v, "collapse", vlen) == 0)
			tag->css.visibility = VIS_COLLAPSE;
		else if (strncmp(v, "hidden", vlen) == 0)
			tag->css.visibility = VIS_HIDDEN;
	}
	else if (strncmp(k, "overflow", klen) == 0) {
		if (strncmp(v, "visible", vlen) == 0) {
			tag->css.overflow_x = OVERFLOW_VISIBLE;
			tag->css.overflow_y = OVERFLOW_VISIBLE;
		}
		else if (strncmp(v, "auto", vlen) == 0) {
			tag->css.overflow_x = OVERFLOW_AUTO;
			tag->css.overflow_y = OVERFLOW_AUTO;
		}
		else if (strncmp(v, "hidden", vlen) == 0) {
			tag->css.overflow_x = OVERFLOW_HIDDEN;
			tag->css.overflow_y = OVERFLOW_HIDDEN;
		}
		else if (strncmp(v, "scroll", vlen) == 0) {
			tag->css.overflow_x = OVERFLOW_SCROLL;
			tag->css.overflow_y = OVERFLOW_SCROLL;
		}
	}
	else if (strncmp(k, "overflow-x", klen) == 0) {
		if (strncmp(v, "visible", vlen) == 0)
			tag->css.overflow_x = OVERFLOW_VISIBLE;
		else if (strncmp(v, "auto", vlen) == 0)
			tag->css.overflow_x = OVERFLOW_AUTO;
		else if (strncmp(v, "hidden", vlen) == 0)
			tag->css.overflow_x = OVERFLOW_HIDDEN;
		else if (strncmp(v, "scroll", vlen) == 0)
			tag->css.overflow_x = OVERFLOW_SCROLL;
	}
	else if (strncmp(k, "overflow-y", klen) == 0) {
		if (strncmp(v, "visible", vlen) == 0)
			tag->css.overflow_y = OVERFLOW_VISIBLE;
		else if (strncmp(v, "auto", vlen) == 0)
			tag->css.overflow_y = OVERFLOW_AUTO;
		else if (strncmp(v, "hidden", vlen) == 0)
			tag->css.overflow_y = OVERFLOW_HIDDEN;
		else if (strncmp(v, "scroll", vlen) == 0)
			tag->css.overflow_y = OVERFLOW_SCROLL;
	}
	else if (strncmp(k, "left", klen) == 0) {
		tag->css.left = atoi(val);
	}
	else if (strncmp(k, "top", klen) == 0) {
		tag->css.top = atoi(val);
	}
	else if (strncmp(k, "right", klen) == 0) {
		tag->css.right = atoi(val);
	}
	else if (strncmp(k, "bottom", klen) == 0) {
		tag->css.bottom = atoi(val);
	}
	else if (strncmp(k, "height", klen) == 0) {
		tag->css.height = atoi(val);
	}
	else if (strncmp(k, "min-height", klen) == 0) {
		tag->css.min_height = atoi(val);
	}
	else if (strncmp(k, "max-height", klen) == 0) {
		tag->css.max_height = atoi(val);
	}
	else if (strncmp(k, "width", klen) == 0) {
		int w = 0;
		int sn = sscanf (val, "%d%%", &w);
		printf ("sn:%d, w:%d\n", sn, w);
		tag->css.width = atoi(val);
	}
	else if (strncmp(k, "min-width", klen) == 0) {
		tag->css.min_width = atoi(val);
	}
	else if (strncmp(k, "max-width", klen) == 0) {
		tag->css.max_width = atoi(val);
	}
	else if (strncmp(k, "max-width", klen) == 0) {
		tag->css.max_width = atoi(val);
	}

	sfree(val);
}

/* 解析属性中的css */
void parseStyleAttr (Tag* tag, char* text, int len)
{
	char* s = text;
	char* end = text+len;
	char endchar = text[len];
	char* k, *v;
	int klen, vlen;

	*end = '\0';

	while (*text) {

		while (*text && (isspace(*text) || *text==';'))
			text++;
		if (!*text) break;

		k = s = text;
		while (*s && *s!=':') s++;
		if (!*s) break;
		klen = s - text;

		for (text = s + 1; *text && isspace(*text); text++);
		if (!*text) break;

		v = text;
		for (s = text; *s && *s!=';'; s++);
		vlen = s - text;

		printf ("style=%s: %s\n", sdumpn(k, klen), sdumpn(v, vlen));
		if (klen > 0 && vlen > 0)
			setstyle (tag, k, klen, v, vlen);

		text = s;
	}

	*end = endchar;
}

void setattr (Tag* tag, char* key, int klen, char* val, int vlen)
{
	char* v;
	if (!tag || !key || !val) return;

	v = sdumpn(val, vlen);

	if (strncmp(key, "class", klen) == 0)
		tag->attr._class = sdumpn(val, vlen);
	else if (strncmp(key, "id", klen) == 0)
		tag->attr.id = sdumpn(val, vlen);
	else if (strncmp(key, "src", klen) == 0)
		tag->attr.src = sdumpn(val, vlen);
	else if (strncmp(key, "href", klen) == 0)
		tag->attr.href = sdumpn(val, vlen);
	else if (strncmp(key, "title", klen) == 0)
		tag->attr.title = sdumpn(val, vlen);
	else if (strncmp(key, "value", klen) == 0)
		tag->attr.value = sdumpn(val, vlen);
	else if (strncmp(key, "checked", klen) == 0)
		tag->attr.checked = true;
	else if (strncmp(key, "disabled", klen) == 0)
		tag->attr.disabled = true;
	// attribute was css
	else if (strncmp(key, "width", klen) == 0)
		tag->css.width = atoi(v);
	else if (strncmp(key, "height", klen) == 0)
		tag->css.height = atoi(v);
	else if (strncmp(key, "style", klen) == 0)
		parseStyleAttr (tag, val, vlen);

	sfree(v);
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
			setattr (tag, k, klen, v, vlen);
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

/* 返回下一个要解析的位置 */
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

	if (!hdc || !dad) return 0;

	top = wrap->top;
	left = wrap->bottom; //起点
	width = wrap->right;

	if (width <= 0)
		return top;

	if (lh == NULL)
		linehead = dad->child;
	if (linehead == NULL)
		return top;

	glforeach (dad, node) {
		tag = (Tag*)node->data;

		if (tag->type == TEXT) {
			//printf ("taglen: %d\n", len);
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
					//printf ("text: %s, linemaxh: %d\n", tag->name, linehead->linemaxh);
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
					//printf ("img linemax: %d\n", linehead->linemaxh);
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
			HDC hdc = GetWindowDC(hwnd);
			RECT rect;
			GetClientRect (hwnd, &rect);
			rect.right -= rect.left;
			rect.bottom = rect.left;
			layout_render (false, hdc, root, &rect, NULL);
			ReleaseDC(hwnd, hdc);
			Sleep(1);
		}
		return 0;

		case WM_ERASEBKGND:
		return 0;

		case WM_PAINT: {
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);
			RECT rect;
			HDC hmem;
			HBITMAP hbmp;
			int w, h;

			GetClientRect (hwnd, &rect);
			hbmp = CreateCompatibleBitmap (hdc, rect.right, rect.bottom);
			hmem = CreateCompatibleDC (hdc);
			SelectObject (hmem, hbmp);

			w = rect.right - rect.left;
			h = rect.bottom - rect.top;

			i32fillrect (hmem, &rect, 0x00ffffff);

			rect.right -= rect.left;
			rect.bottom = rect.left;
			layout_render (true, hmem, root, &rect, NULL);

			BitBlt (hdc, 0, 0, w, h, hmem, 0, 0, SRCCOPY);

			DeleteObject(hbmp);
			DeleteObject(hmem);
			EndPaint(hwnd, &ps);
			Sleep(1);
		}
		return 0;
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

	buf = fdump ("test.htm");
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
