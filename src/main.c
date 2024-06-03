#include <SDL.h>
#include <SDL_image.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <stdint.h>

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#include "lua_functions.h"
#include "main.h"
#include "draw_functions.h"

// Screen dimensions
const int SCREEN_WIDTH = 512;
const int SCREEN_HEIGHT = 512;

SDL_Renderer* renderer;
SDL_Texture* spritesheet;
SDL_Window* window;
lua_State* L;

int main(int argc, char* argv[]) {

    window = SDL_CreateWindow("SDL Random Colors with Image Overlay", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_RESIZABLE);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    spritesheet = IMG_LoadTexture(renderer, "assets/Untitled.png");

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    // set up lua VM and functions
    L = luaL_newstate();
    luaL_openlibs(L);

    lua_pushinteger(L, SCREEN_WIDTH); 
    lua_setglobal(L, "SCREEN_WIDTH");
    lua_pushinteger(L, SCREEN_HEIGHT);
    lua_setglobal(L, "SCREEN_HEIGHT");
    lua_pushcfunction(L, lua_sprite);
    lua_setglobal(L, "sprite");
    lua_pushcfunction(L, lua_millis);
    lua_setglobal(L, "millis");
    lua_pushcfunction(L, lua_stroke);
    lua_setglobal(L, "stroke");
    lua_pushcfunction(L, lua_fill);
    lua_setglobal(L, "fill");
    lua_pushcfunction(L, lua_rect);
    lua_setglobal(L, "rect");
    lua_pushcfunction(L, lua_pset);
    lua_setglobal(L, "pset");


    // inital state
    luaL_dofile(L, "assets/script.lua");

    // check if the draw function is found
    lua_getglobal(L, "_draw");
    if (lua_isfunction(L, -1)) {
        if (lua_pcall(L, 0, 1, 0) == LUA_OK) {
            lua_pop(L, lua_gettop(L));
        }
    }
    else {
        printf("draw function not found");
        destroyApplication();
        return 1;
    }

    srand((unsigned int)time(NULL));
    bool running = true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderClear(renderer);

        // execute lua draw function
        lua_getglobal(L, "_draw");
        if(lua_pcall(L, 0, 1, 0) == LUA_OK) {
            lua_pop(L, lua_gettop(L));
        } else {
            destroyApplication();
            return 1;
        }

        SDL_RenderPresent(renderer);
    }

    destroyApplication();

    return 0;
}

void destroyApplication() {
    lua_close(L);
    SDL_DestroyTexture(spritesheet);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
}
