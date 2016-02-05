#include <stdio.h>
#include <string.h>
#include "lua/src/lua.h"
#include "lua/src/lualib.h"
#include "lua/src/lauxlib.h"

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <Glut/glut.h>
#else
#include <GL/gl.h>
#include <GL/glut.h>
#endif

void DoNothing()
{

}

void ReSizeWindow( int with, int height )
{

}

int CreateWindow(lua_State* L)
{
  
  glutInitWindowSize(300, 300);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutCreateWindow("es2gears");
     glutIdleFunc (DoNothing);
   glutReshapeFunc(ReSizeWindow);
   glutDisplayFunc(DoNothing);

  return 0;
}


int main (int argc, char *argv[])
{
  glutInit(&argc, argv);
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
    glutMainLoop();
  }
  
  

  lua_close(L);
  return 0;
}
