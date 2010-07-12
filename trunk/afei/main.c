#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <windows.h>

void strrplc(char*s, char c, char dst)
{
    char *pos = strchr(s, c);
    while (pos) {
        *pos = dst;
        pos = strchr(s, c);
    }
}

char *filedup (char* filename)
{
    FILE* f;
    char* buf;
    int filesize;

    f = fopen(filename, "rb");
    if (!f) {
        printf("file:%s not found!\n", filename);
        exit(1);
    }
    fseek(f, 0L, SEEK_END);
    filesize = ftell(f);
    fseek(f, 0L, SEEK_SET);
    buf = (char*)malloc(filesize+1);
    fread(buf, 1, filesize, f);
    buf[filesize] = '\0';
    fclose(f);
    return buf;
}

/*
@key-value���ݿ��ļ���ʽ:
key:value;
key:value;
...

@ÿ����������ݽṹ:
�̶���С���ַ���ָ������,��key,value,key,value...�ķ�ʽ����,
��NULL��β,�����򵥡�oҪ�ȳ�ʼ��Ϊ��. �����ж���������.
*/
int loaddb (char* path, char*o[])
{
    char* buf;
    int i = 0;
    char* tok;

    buf = filedup(path);
    tok = strtok(buf, ";");
    while (tok) {
        char* key;
        char* value;
        char* spt = strchr(tok, ':');
        if (spt) {
            *spt = '\0';
            for(key = tok; isspace(*key); key++); //���˿ո�ͻ���
            key = strdup(key);
            value = strdup(spt+1);
            o[i*2] = key;
            o[i*2+1] = value;
            i++;
            *spt = ':'; //rollback
        }
        //printf ("%s", tok);
        tok = strtok(NULL, ";");
    }
    free(buf);
    return i;
}

void savedb (char*db[], char* path)
{
    FILE* f = fopen(path, "w");
    char** p;

    for (p = db; *p; p+=2)
        fprintf (f, "%s: %s;\n", *p, *(p+1));
    fclose(f);
}

/* @����key��������NULL */
char* dbget(char*db[], char* key)
{
    char**p;

    for (p = db; *p; p+=2)
        if (strcmp(*p, key)==0)
            return *(p+1);
    return NULL;
}

/* @����key��value��������NULL */
void dbset(char*db[], char* key, char* value)
{
    char** p;
    for (p = db; *p; p+=2)
        if (strcmp(*p, key)==0) {
            free(*(p+1));
            *(p+1) = strdup(value);
            return;
        }
    *p = strdup(key);
    *(p+1) = strdup(value);
}

int dbcount(char*db[])
{
    int n = 0;
    char**p;
    for (p = db; *p; p+=2)
        n++;
    return n;
}

void printdb (char*db[])
{
    char **p;
    for (p = db; *p; p+=2)
        printf ("%s: %s;\n", *p, *(p+1));
}

void freedb(char*map[])
{
    char **p;
    for (p = map; *p; p++) {
        free(*p);
        *p = NULL;
    }
}

/*****************  LALA����  *****************/

char* g_map[100] = {NULL}; //��ǰ��ͼ
char* g_me[100] = {NULL}; //��������
char* g_cmdline[100] = {NULL}; //ÿ��ָ��Ľű����б�
int g_lineno = 0; //�к�
int g_ret = 0; //��һ�����µķ���ֵ

void gotomap(char* mapname);

/* �̳�loaddb */
int loadmap(char* mapname, char*data[])
{
    char path[1024];

    sprintf (path, "map/%s", mapname);
    strrplc(path, '.', '/');

    return loaddb(path, data);
}

/* ִ��Ԫ���ʽ,˳�������,����-1,0,1 */
int doexp(char* s)
{
	char* op = strtok(s, " \t");
	char *p1, *p2;

	if (!op) return -1;

	// ȥĳ��ͼ
	if (strcmp(op, "GOTO")==0) {
		p1 = strtok(NULL, " \t");
		if (!p1) {
			printf ("GOTO where?");
			return -1;
		}
		gotomap(p1);
		return -1;
	}
	// ��ӡ����,��õ���ʹ��.
	if (strcmp(op, "!")==0) {
		p1 = op+strlen(op)+1;
		printf ("%s\n", p1);
		return -1;
	}
	// �����������Ա���
	if (strcmp(op, "SET")==0) {
		p1 = strtok(NULL, " \t");
		if (!p1) {
			printf ("error: operator 'SET' need a key.");
			exit(1);
		}
		p2 = strtok(NULL, " \t");
		if (!p2) {
			printf ("error: operator 'SET' need a value.");
			exit(1);
		}
		dbset(g_me, p1, p2);
		printdb(g_me);
		return -1;
	}
	// �Ƚ����Ա���
	if (strcmp(op, ">")==0) {
		char* vs;
		int v, i2;
		p1 = strtok(NULL, " \t");
		if (!p1) {
			printf ("error: operator '>' need a variable-name to compare.");
			exit(1);
		}
		p2 = strtok(NULL, " \t");
		vs = dbget(g_me, p1);
		if (!vs) return 0;
		if (p2==NULL) i2 = 0;
		else i2 = atoi(p2);
		v = atoi(vs);
		if (v > i2) return 1;
		return 0;
	}
	if (strcmp(op, ">=")==0) {
		char* vs;
		int v, i2;
		p1 = strtok(NULL, " \t");
		if (!p1) {
			printf ("error: operator '>=' need a variable-name to compare.");
			exit(1);
		}
		p2 = strtok(NULL, " \t");
		vs = dbget(g_me, p1);
		if (!vs) v = 0;
		else v = atoi(vs);
		if (p2==NULL) i2 = 0;
		else i2 = atoi(p2);
		if (v >= i2) return 1;
		return 0;
	}
	if (strcmp(op, "=")==0) {
		char* vs;
		int v, i2;
		p1 = strtok(NULL, " \t");
		if (!p1) {
			printf ("error: operator '=' need a variable-name to compare.");
			exit(1);
		}
		p2 = strtok(NULL, " \t");
		vs = dbget(g_me, p1);
		if (!vs) v = 0;
		else v = atoi(vs);
		if (p2==NULL) i2 = 0;
		else i2 = atoi(p2);
		if (v == i2) return 1;
		return 0;
	}
	if (strcmp(op, "<")==0) {
		char* vs;
		int v, i2;
		p1 = strtok(NULL, " \t");
		if (!p1) {
			printf ("error: operator '<' need a variable-name to compare.");
			exit(1);
		}
		p2 = strtok(NULL, " \t");
		vs = dbget(g_me, p1);
		if (!vs) v = 0;
		else v = atoi(vs);
		if (p2==NULL) i2 = 0;
		else i2 = atoi(p2);
		if (v < i2) return 1;
		return 0;
	}
	if (strcmp(op, "<=")==0) {
		char* vs;
		int v, i2;
		p1 = strtok(NULL, " \t");
		if (!p1) {
			printf ("error: operator '<=' need a variable-name to compare.");
			exit(1);
		}
		p2 = strtok(NULL, " \t");
		vs = dbget(g_me, p1);
		if (!vs) v = 0;
		else v = atoi(vs);
		if (p2==NULL) i2 = 0;
		else i2 = atoi(p2);
		if (v <= i2) return 1;
		return 0;
	}
	return -1;
}

/* ִ����ϱ��ʽ(�ں�&��|),��ʽ�޸�g_ret��ֵ */
void dostring(char* s)
{
	char* tok;
	char* p;
	char op;
	int ret;
	if (!strchr(s, '|') && !strchr(s, '&')) {
		ret = doexp(s);
		if (ret >= 0)
			g_ret = ret;
		return;
	}

	for (p = s; *p!='&'&&*p!='|'; p++);
	op = *p;
	*p = '\0';
	ret = doexp(s);

	for (s = p+1; *s; s = p+1) {
		char tmpop;
		int r;
		for (p = s; *p&&*p!='&'&&*p!='|'; p++);
		tmpop = *p;
		*p = '\0';

		r = doexp(s);
		if (op == '|')
			ret = ret || r;
		else if (op == '&')
			ret = ret && r;

		if (tmpop)
			op = tmpop;
		else
			break;
	}
	g_ret = ret!=0;
}

/* ִ��һ������,��һ�еĵ�һ����Ӧ����һ��LALA�����ڶ����Ʒ�(����?~.&|),���򶼰����ʽִ��. */
void doline(char* line)
{
	char* linebuf = strdup(line);
	char* op = strtok(linebuf, " \t");
	char* param = op+strlen(op)+1;
	int ret = g_ret;

	if (strcmp(op, "?")==0) {
		if(ret) dostring(param);
	}
	else if (strcmp(op, "~")==0) {
		if(!ret) dostring(param);
	}
	else if (strcmp(op, ".")==0)
		g_ret = 0;
	else if (strcmp(op, "&")==0) {
		dostring(param);
		g_ret = ret && g_ret;
	}
	else if (strcmp(op, "|")==0) {
		dostring(param);
		g_ret = ret || g_ret;
	}
	else {
		param[-1] = ' '; //������
		dostring(linebuf);
	}
	free(linebuf);
}

/* create tmp line-buffer, �������� */
int loadcmdline (char* script)
{
	int n = 0;
	//create tmp line-buffer
	char* tok = strtok(script, "\r\n");
	while (tok) {
		g_cmdline[n++] = strdup(tok);
		tok = strtok(NULL, "\r\n");
	}
	return n;
}

void freecmdline ()
{
	int i;
	char** p;
	for (i = 0; i < sizeof(g_cmdline)/sizeof(g_cmdline[0]); i++) {
		p = g_cmdline+i;
		if (*p)
			free(*p);
	}
	memset(g_cmdline, 0, sizeof(g_cmdline));
	g_lineno = 0;
}

/* ִ��һ���ű�
   �ű�����Ϊ��λ,һ��һ��ִ��,
   ֱ������������� */
void doscript (char* script)
{
	int n = 0, i;

	n = loadcmdline(script);
	//for (i = 0; i < n; i++) printf ("#%d: %s\n", i, g_cmdline[i]);
	//interpretate line by line
	if (n > 0) {
		for(g_lineno = 0; g_lineno < n; g_lineno++)
			doline(g_cmdline[g_lineno]);
	}

	freecmdline();
}

/* һ������ǻ��ڵ�ǰ��ͼ�� */
void mapcmd (char* cmd)
{
    char* v = dbget(g_map, cmd);
    char* script;

	if (strcmp(cmd, "q")==0)
		exit(0);

    if (!v) {
        printf ("?\n");
        return;
    }

    script = strdup(v);
    doscript(script);
    free(script);
}

/* �ٽ����� */
void delaybar(int sec)
{
    int i;
    for (i = 0; i < sec; i++) {
        printf (".");
        Sleep(600);
    }
    printf ("\n\n");
}

/* ��ͼ�������õ�����ʾ�ӵ�ͼ, ����cp1.snowroad1*/
void gotomap(char* mapname)
{
    freedb(g_map);
    loadmap(mapname, g_map);
    freecmdline();
    //delaybar(3);
    mapcmd("look");
}



int main()
{
    char cmd[20];

    //printf ("����絶���Դ��Ϊ��壬������Ϊ���⡣\n�����ѩ�����������¯��������Ϊ������\n\n");
    gotomap("cp1.test1");
    //printdb(map);

    while (1) {
        gets(cmd);
        mapcmd(cmd);
    }

    //freedb(g_map);

    return 0;
}
