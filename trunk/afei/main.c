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

/**********************************/

char* g_map[100] = {NULL}; //��ǰ��ͼ

void gotomap(char* mapname);

/* �̳�loaddb�ķ�֧���� */
int loadmap(char* mapname, char*data[])
{
    char path[1024];

    sprintf (path, "map/%s", mapname);
    strrplc(path, '.', '/');

    return loaddb(path, data);
}

void doscript (char* s)
{
    char* cmd = strtok(s, " \t\r\n");
    char* param = cmd?cmd+strlen(cmd)+1:"(empty param)";
    //ECHO
    if (strcmp(cmd, "!")==0)
        printf("%s\n", param);
    else if (strcmp(cmd, "GOTO")==0)
        gotomap(param);
        //printf("%s\n", param);
    else
        printf("wrong command:%s\n", cmd);
}

void mapcmd (char* cmd)
{
    char* v = dbget(g_map, cmd);
    char* script;
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
        Sleep(900);
    }
    printf ("\n\n");
}

/* ��ͼ��������.����ʾ�ӵ�ͼ */
void gotomap(char* mapname)
{
    freedb(g_map);
    loadmap(mapname, g_map);
    delaybar(3);
    mapcmd("look");
}



int main()
{
    char cmd[20];

    printf ("����絶���Դ��Ϊ��壬������Ϊ���⡣\n�����ѩ�����������¯��������Ϊ������\n\n");

    gotomap("cp1.snowroad1");
    //printdb(map);

    while (1) {
        gets(cmd);
        if (strcmp(cmd, "q")==0)
            return 0;
        mapcmd(cmd);
    }

    //freedb(g_map);

    return 0;
}
