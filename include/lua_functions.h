#ifndef LUA_FUNCTIONS_H
#define LUA_FUNCTIONS_H

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

void lua_sprite(lua_State* L);
int lua_millis(lua_State* L);
int lua_stroke(lua_State* L);
int lua_fill(lua_State* L);
int lua_rect(lua_State* L);

#endif