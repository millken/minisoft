#include <windows.h>
#include "i32.h"
#include "ds.h"

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

typedef enum {
	TEXT,
	ELEMENT
} TagType;

typedef struct {
	TagType type;
	char* name;
	char* attr;
} Tag;

Tag* newtag (TagType type)
{
	Tag* tag = (Tag*)salloc(sizeof(Tag));
	tag->type = type;
	return tag;
}

void deltag (Tag* tag)
{
	if (tag) {
		sfree(tag->name);
		sfree(tag->attr);
		free(tag);
	}
}

char* lex (const char* text, glnode* dad)
{
	glnode* node;
	Tag* tag;
	char* s, *dadname;

	if (!dad || !dad->data || !text || *text=='\0')
		return NULL;

	tag = (Tag*)dad->data;
	dadname = tag->name ? tag->name : "";

	// loop brothers
	while (true) {
		// parse text, go to '<'
		for (s = text; *s && *s!='<'; s++);
		if (s > text) {
			tag = newtag (TEXT);
			tag->name = sdumpn (text, s - text);

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
			tag = newtag(ELEMENT);
			tag->name = sdumpn(text, s-text);
			node = glnew(tag);
			gljoin (dad, node, -1);

			//printf (":%s\n", s+1);
			text = lex(s+1, node);

			if (!text) return NULL;
		}

		if (*text == '\0') return text;
		//text = s + 1;
	}

	return text;
}

void print_gl (glnode* node, int indent)
{
	glnode* p;
	Tag* tag;
	int i;

	if (!node) return;

	glforeach (node, p) {
		if (p && p->data) {
			tag = (Tag*)p->data;
			for (i = 0; i < indent; i++)
				printf (">");
			printf ("[%d]%s\n", tag->type, tag->name);
			print_gl(p, indent+1);
		}
	}
}

int WINAPI WinMain (HINSTANCE hithis, HINSTANCE hipre, PSTR param, int icmd)
{
	char* buf;
	glnode* root;

	buf = fdump ("index.htm");
	root = glnew(newtag(ELEMENT));
	lex (buf, root);

	print_gl (root, 0);



	sfree(buf);

	return 0;
}
