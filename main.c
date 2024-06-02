#include <SDL.h>
#include <SDL_image.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <stdint.h>

#include "lua/lua.h"
#include "lua/lualib.h"
#include "lua/lauxlib.h"

#include "lua_functions.h"
#include "main.h"
#include "draw_functions.h"

// Screen dimensions
const int SCREEN_WIDTH = 512;
const int SCREEN_HEIGHT = 512;

SDL_Renderer* renderer;
SDL_Texture* background;
SDL_Texture* spritesheet;

int main(int argc, char* argv[]) {

    SDL_Window* window = SDL_CreateWindow("SDL Random Colors with Image Overlay", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    background = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
    spritesheet = IMG_LoadTexture(renderer, "Untitled.png");

    // set up lua VM and functions
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);
    lua_pushcfunction(L, lua_draw_sprite);
    lua_setglobal(L, "draw_sprite");
    lua_pushcfunction(L, lua_draw_sprite_advanced);
    lua_setglobal(L, "draw_sprite_advanced");
    lua_pushcfunction(L, lua_millis);
    lua_setglobal(L, "millis");
    luaL_dofile(L, "script.lua");

    srand((unsigned int)time(NULL));
    bool running = true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        SDL_RenderClear(renderer);

        void* pixels;
        int pitch;
        SDL_LockTexture(background, NULL, &pixels, &pitch);

        Uint32* pixelData = (Uint32*)pixels;
        for (int y = 0; y < SCREEN_HEIGHT; ++y) {
            for (int x = 0; x < SCREEN_WIDTH; ++x) {
                Uint8 r = x % 256;
                Uint8 g = y % 256;
                Uint8 b = 0;
                pixelData[y * (pitch / 4) + x] = (r << 24) | (g << 16) | (b << 8) | 255;
            }
        }

        int startX = 50;
        int startY = 100;

        SDL_UnlockTexture(background);

        SDL_RenderCopy(renderer, background, NULL, NULL);

        luaL_dofile(L, "script.lua");

        SDL_RenderPresent(renderer);
    }

    lua_close(L);
    SDL_DestroyTexture(spritesheet);
    SDL_DestroyTexture(background);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();

    return 0;
}
