
#include <stdio.h>
#include <string.h>
#include "lua/src/lua.h"
#include "lua/src/lualib.h"
#include "lua/src/lauxlib.h"

#include <emscripten/emscripten.h>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <Glut/glut.h>
#else
#include <GL/glfw.h>
#include <GL/gl.h>
#endif

void ReSizeWindow( int with, int height )
{

}

int CreateWindow(lua_State* L)
{

    const int width = 480,
             height = 800;
 
    if (glfwInit() != GL_TRUE) {
        printf("glfwInit() failed\n");
        return 0;
    }
 
    if (glfwOpenWindow(width, height, 8, 8, 8, 8, 16, 0, GLFW_WINDOW) != GL_TRUE) {
        printf("glfwOpenWindow() failed\n");
        return 0;
    }
 
    return 1;
}

void do_frame()
{
}

int main (int argc, char *argv[])
{
//  glutInit(&argc, argv);
  int error;
  lua_State *L = luaL_newstate();   /* opens Lua */
  luaL_openlibs(L); /*open the lua libs*/


   /* Set up glut callback functions */

   //glutSpecialFunc(DoNothing);


  //Register Create Window Function
  lua_register(L, "CreateWindow", CreateWindow);

  auto buff = "io.write(\"Hello World\");\nCreateWindow();";

  error = luaL_loadbuffer(L, buff, strlen(buff), "line") || lua_pcall(L, 0, 0, 0);
  if (error) 
  {
    fprintf(stderr, "%s", lua_tostring(L, -1));
    lua_pop(L, 1);  /* pop error message from the stack */
  }
  else
  {
	  emscripten_set_main_loop(do_frame, 0, 1);
	  // glutMainLoop();
  }
  
  

  lua_close(L);
  return 0;
}
