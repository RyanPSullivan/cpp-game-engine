#include "lua/src/lua.h"
#include "lua/src/lualib.h"
#include "lua/src/lauxlib.h"
#include "luagl.h"


#if EMSCRIPTEN
#include <emscripten/emscripten.h>
#else
#define USE_GLEW 1
#endif

#include "SDL/SDL.h"

#if USE_GLEW
#include "GL/glew.h"
#else
#include "SDL/SDL_opengl.h"
#endif

#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <assert.h>

int CreateWindow(lua_State* L)
{
    SDL_Surface *screen;
    if ( SDL_Init(SDL_INIT_VIDEO) != 0 ) {
        printf("Unable to initialize SDL: %s\n", SDL_GetError());
        return 1;
    }

    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
    screen = SDL_SetVideoMode( 640, 480, 24, SDL_OPENGL );
    if ( !screen ) {
        printf("Unable to set video mode: %s\n", SDL_GetError());
        return 1;
    }

#if USE_GLEW
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
	    /* Problem: glewInit failed, something is seriously wrong. */
	    fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
	    exit(1);
    }
    fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
#endif

    return 0;
}


int time_to_next_frame()
{
	return 16;
}

void init()
{

}

void update()
{
  //invoke lua
  SDL_GL_SwapBuffers();
}

static int traceback(lua_State *L) {
  lua_getglobal(L, "debug");
    lua_getfield(L, -1, "traceback");
    lua_pushvalue(L, 1);
    lua_pushinteger(L, 2);
    lua_call(L, 2, 1);
    fprintf(stderr, "%s\n", lua_tostring(L, -1));
    return 1;
}


int loadLua( lua_State* L, const char* path )
{
  //load lua scripts
  FILE *file = fopen(path, "rb");

  if (file==NULL)
  {
    fputs ("File error",stderr);
    return 1;
  }

  // obtain file size:
  fseek (file , 0 , SEEK_END);
  auto lSize = ftell (file);
  rewind (file);

  // allocate memory to contain the whole file:
  auto buffer = (char*) malloc (sizeof(char)*lSize);
  if (buffer == NULL)
  {
    fputs ("Memory error",stderr);
    return 2;
  }

  // copy the file into the buffer:
  auto result = fread (buffer,1,lSize,file);
  if (result != lSize)
  {
    fputs ("Reading error", stderr);
    return 3;
  }

  //close the file its now loaded into a memory buffer
  fclose (file);

  int error = luaL_loadbuffer(L, buffer, strlen(buffer), path);
  if (error)
  {
    fprintf(stderr, "loadbuffer %s", lua_tostring(L, -1));
    lua_pop(L, 1);  /* pop error message from the stack */
    return -1;
  }

  //The buffer has been handed off to lua now
  free(buffer);
  return 0;
}

int main (int argc, char *argv[])
{
  lua_State *L = luaL_newstate();   /* opens Lua */
  luaL_openlibs(L); /*open the lua libs*/
  luaL_opengl(L);
  lua_pushcfunction(L, traceback);

  //Register Create Window Function
  lua_register(L, "CreateWindow", CreateWindow);

  int error =   loadLua(L,"lua/draw.lua");
  if (error)
  {
    return error;
  }

  error = lua_pcall(L, 0, 0, lua_gettop(L) - 1);
  if (error)
  {
    fprintf(stderr, "pcall %s", lua_tostring(L, -1));
    lua_pop(L, 1);  /* pop error message from the stack */
    return -1;
  }



#if EMSCRIPTEN
	  emscripten_set_main_loop(onUpdate, 0, 1);
#else
  SDL_Event e;
  bool quit = false;

  while (!quit){
      onUpdate();
      while (SDL_PollEvent(&e)){
          if (e.type == SDL_QUIT){
              quit = true;
          }
          if (e.type == SDL_KEYDOWN){
              quit = true;
          }
          if (e.type == SDL_MOUSEBUTTONDOWN){
              quit = true;
          }
      }
  }
#endif

  SDL_Quit();

  lua_close(L);
  return 0;
}
