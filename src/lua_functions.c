
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#include <SDL.h>
#include <SDL_image.h>

#include "draw_functions.h"

void lua_draw_sprite(lua_State* L) {
    int sourceX = (int)luaL_checknumber(L, 1);
    int sourceY = (int)luaL_checknumber(L, 2);
    int sourceW = (int)luaL_checknumber(L, 3);
    int sourceH = (int)luaL_checknumber(L, 4);

    int targetX = (int)luaL_checknumber(L, 5);
    int targetY = (int)luaL_checknumber(L, 6);
    int targetW = (int)luaL_checknumber(L, 7);
    int targetH = (int)luaL_checknumber(L, 8);

    draw_sprite(sourceX, sourceY, sourceW, sourceH, targetX, targetY, targetW, targetH);
}

void lua_draw_sprite_advanced(lua_State* L) {
    int sourceX = (int)luaL_checknumber(L, 1);
    int sourceY = (int)luaL_checknumber(L, 2);
    int sourceW = (int)luaL_checknumber(L, 3);
    int sourceH = (int)luaL_checknumber(L, 4);

    int targetX = (int)luaL_checknumber(L, 5);
    int targetY = (int)luaL_checknumber(L, 6);
    int targetW = (int)luaL_checknumber(L, 7);
    int targetH = (int)luaL_checknumber(L, 8);

    int angle = (int)luaL_checknumber(L, 9);
    int flip = (int)luaL_checknumber(L, 10);

    draw_sprite_advanced(sourceX, sourceY, sourceW, sourceH, targetX, targetY, targetW, targetH, angle, (SDL_RendererFlip)flip);
}

int lua_millis(lua_State* L) {
    lua_Integer m = millis();
    lua_pushinteger(L, m);
    return 1;
}