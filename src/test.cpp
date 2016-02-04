#include <stdio.h>
#include <string.h>
#include "lua/src/lua.h"
#include "lua/src/lualib.h"
#include "lua/src/lauxlib.h"

int main (void) 
{
  int error;
  lua_State *L = luaL_newstate();   /* opens Lua */
  luaL_openlibs(L); /*open the lua libs*/
  
  auto buff = "io.write(\"Hello World\")";

  error = luaL_loadbuffer(L, buff, strlen(buff), "line") || lua_pcall(L, 0, 0, 0);
  if (error) 
  {
    fprintf(stderr, "%s", lua_tostring(L, -1));
    lua_pop(L, 1);  /* pop error message from the stack */
  }
  
  lua_close(L);
  return 0;
}
