#ifndef LUA_FUNCTIONS_H
#define LUA_FUNCTIONS_H

#include "lua/lua.h"
#include "lua/lualib.h"
#include "lua/lauxlib.h"

void lua_draw_sprite(lua_State* L);
void lua_draw_sprite_advanced(lua_State* L);
int lua_millis(lua_State* L);

#endif