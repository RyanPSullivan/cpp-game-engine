//A lua wrapper around opengl methods
#ifndef __EMSCRIPTEN__
#define USE_GLEW 1
#endif

#if USE_GLEW
#include "GL/glew.h"
#endif

#include "SDL/SDL.h"

#if !USE_GLEW
#include "SDL/SDL_opengl.h"
#endif

#include "lua/src/lua.h"
#include "lua/src/lauxlib.h"
#include "lua/src/lualib.h"

static int glClearColor(lua_State *L)
{
	glClearColor(
		luaL_checknumber(L, 1),
		luaL_checknumber(L, 2),
		luaL_checknumber(L, 3),
		luaL_checknumber(L, 4));

	return 0;
}

static int glClear(lua_State *L)
{
	glClear(luaL_checkinteger(L, 1));
	return 0;
}

static int gl_GenVertexArrays(lua_State *L)
{
	auto num  = luaL_checkinteger(L, 1);
	
	GLuint* arrays = new GLuint[num];
	glGenVertexArrays(num,arrays);

	  lua_newtable(L);
	  for(int i = 0;i < num;i++)
	  {
		  lua_pushinteger(L,arrays[i]);
		  lua_rawseti(L,-2,i + 1);
	  }
	delete [] arrays;
	return 1;
}

static const luaL_Reg gllib[] = {
  {"glClearColor",   glClearColor},
  {"glClear", glClear},
  {"glGenVertexArrays", gl_GenVertexArrays},
  {NULL, NULL}
};

/*
** Open GL library
*/
LUAMOD_API int luaopen_gl (lua_State *L) {
  luaL_newlib(L, gllib);

  return 1;
}
	
