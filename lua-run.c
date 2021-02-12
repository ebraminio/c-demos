// clang lua-run.c `pkg-config --libs --cflags lua` -Wall && ./a.out
// https://lucasklassmann.com/blog/2019-02-02-how-to-embeddeding-lua-in-c/
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

int multiplication(lua_State *L) {
  int a = luaL_checkinteger(L, 1);
  int b = luaL_checkinteger(L, 2);
  lua_Integer c = a * b;
  lua_pushinteger(L, c);
  return 1;
}

int main(int argc, char **argv) {
  lua_State *L = luaL_newstate();
  luaL_openlibs(L);
  lua_newtable(L);
  const struct luaL_Reg my_functions_registery[] = {{"mul", multiplication}};
  // put the registry on the stack
  luaL_setfuncs(L, my_functions_registery, 0);
  // set it to my_functions_registry
  lua_setglobal(L, "my_functions_registry");
  char *code = "print(my_functions_registry.mul(7, 8))";

  if (luaL_loadstring(L, code) == LUA_OK) {
    if (lua_pcall(L, 0, 1, 0) == LUA_OK) {
      lua_pop(L, lua_gettop(L));
    }
  }

  lua_close(L);
  return 0;
}