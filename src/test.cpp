#include <stdio.h>
#include <string>
#include "duktape/duktape.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/bind.h>
#include <SDL/SDL.h>
#endif

void executeJS( std::string thing )
{
  duk_context *ctx = duk_create_heap_default();
  duk_eval_string(ctx, thing.c_str());
  duk_destroy_heap(ctx);
}

#ifdef __EMSCRIPTEN__
using namespace emscripten;

EMSCRIPTEN_BINDINGS(what)
{
   function("executeJS", &executeJS);
}
#endif

void one_iter() {
  // process input
  // render to screen
    printf("WHA\n");
}

extern "C" int main(int argc, char** argv) {
  printf("hello, world!\n");

  executeJS( "print('22+20');" );

#ifdef __EMSCRIPTEN__
   emscripten_set_main_loop(one_iter, 60, 1);
#endif

  return 0;
}




