#include <stdio.h>

#include "duktape/duktape.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <SDL/SDL.h>
#endif

void one_iter() {
  // process input
  // render to screen
	printf("WHA");
}

extern "C" int main(int argc, char** argv) {
  printf("hello, world!\n");

  duk_context *ctx = duk_create_heap_default();
  duk_eval_string(ctx, "print('22+20');");
  duk_destroy_heap(ctx);

#ifdef __EMSCRIPTEN__
   emscripten_set_main_loop(one_iter, 60, 1);
#endif

  return 0;
}


