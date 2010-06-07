#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>


static void absdir (char* path, int size)
{
	int len, i;

	len = GetModuleFileNameA (NULL, path, size);
	for (i = len; i >=0 && path[i]!='/' && path[i]!='\\'; i--);
	path[i] = '\0';
}

/* 给lua注册一个Sleep函数 */
static int l_sleep (lua_State *L)
{
    double d = lua_tonumber(L, 1);  /* get argument */
	Sleep((int)d);
    return 1;                      /* number of results */
}

int main(int argn, char** arg)
{
	char *script;
	char path[1024];
	int error;

	lua_State *L;

	script = getenv("SCRIPT_FILENAME");
	if (!script) {
		printf ("no ENV[SCRIPT_FILENAME]<br>\n\n");
		return 0;
	}

	L = lua_open ();
	luaL_openlibs(L);

	lua_pushcfunction(L, l_sleep);
	lua_setglobal(L, "Sleep");

	absdir (path, sizeof(path));
	sprintf (path, "%s\\%s", path, script);
	error = luaL_loadfile(L, path) || lua_pcall(L, 0, LUA_MULTRET, 0);
	if (error) {
		printf ("Content-type: text/html\n\n");
		printf ("<b>Lua Error:</b> %s<br/>\n", lua_tostring(L, -1));
		lua_pop(L, 1);
		return 0;
	}

	lua_close (L);

    return 0;
}
