#ifndef LUA_FUNCTIONS_H
#define LUA_FUNCTIONS_H

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

void lua_setup_functions();

int lua_sprite(lua_State* L);
int lua_millis(lua_State* L);
int lua_stroke(lua_State* L);
int lua_fill(lua_State* L);
int lua_rect(lua_State* L);
int lua_pset(lua_State* L);
int lua_tone(lua_State* L);
int lua_bpm(lua_State* L);
int lua_btn(lua_State* L);

#endif