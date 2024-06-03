
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#include <SDL.h>
#include <SDL_image.h>

#include "draw_functions.h"

void lua_sprite(lua_State* L) {
    int sourceX = (int)luaL_checknumber(L, 1);
    int sourceY = (int)luaL_checknumber(L, 2);
    int sourceW = (int)luaL_checknumber(L, 3);
    int sourceH = (int)luaL_checknumber(L, 4);

    int targetX = (int)luaL_checknumber(L, 5);
    int targetY = (int)luaL_checknumber(L, 6);
    int targetW = (int)luaL_checknumber(L, 7);
    int targetH = (int)luaL_checknumber(L, 8);

    if (lua_gettop(L) == 8) {
        draw_sprite(sourceX, sourceY, sourceW, sourceH, targetX, targetY, targetW, targetH);
        return;
    }

    int angle = (int)luaL_checknumber(L, 9);
    int flip = luaL_checkinteger(L, 10);

    if (lua_gettop(L) == 10) {
        draw_sprite_advanced(sourceX, sourceY, sourceW, sourceH, targetX, targetY, targetW, targetH, angle, (FLIP)flip);
        return;
    }
}

int lua_millis(lua_State* L) {
    lua_Integer m = millis();
    lua_pushinteger(L, m);
    return 1;
}

void lua_stroke(lua_State* L) {
    if (lua_gettop(L) != 5) {
        return;
    }

    int width = (int)luaL_checknumber(L, 1);
    int r = (int)luaL_checknumber(L, 2);
    int g = (int)luaL_checknumber(L, 3);
    int b = (int)luaL_checknumber(L, 4);
    int a = (int)luaL_checknumber(L, 5);

    set_stroke(width, r, g, b, a);
}

void lua_fill(lua_State* L) {
    if (lua_gettop(L) != 4) {
        return;
    }

    int r = (int)luaL_checknumber(L, 1);
    int g = (int)luaL_checknumber(L, 2);
    int b = (int)luaL_checknumber(L, 3);
    int a = (int)luaL_checknumber(L, 4);

    set_fill(r, g, b, a);
}

void lua_rect(lua_State* L) {
    if (lua_gettop(L) != 4) {
        return;
    }

    int x = (int)luaL_checknumber(L, 1);
    int y = (int)luaL_checknumber(L, 2);
    int w = (int)luaL_checknumber(L, 3);
    int h = (int)luaL_checknumber(L, 4);

    draw_rect(x, y, w, h);
}

