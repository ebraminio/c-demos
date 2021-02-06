// gcc mujs-run.c `pkg-config --libs --cflags mujs` -Wall && ./a.out
// https://mujs.com/examples.html
#include <stdio.h>
#include <mujs.h>

static void hello(js_State *J) {
  const char *name = js_tostring(J, 1);
  printf("Hello, %s!\n", name);
  js_pushundefined(J);
}

int main(int argc, char **argv) {
  js_State *J = js_newstate(NULL, NULL, JS_STRICT);

  js_newcfunction(J, hello, "hello", 1);
  js_setglobal(J, "hello");

  js_dostring(J, "hello('world');");

  js_freestate(J);
}
