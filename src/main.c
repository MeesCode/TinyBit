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
#include "graphics.h"
#include "audio.h"
#include "input.h"
#include "memory.h"

SDL_Renderer* renderer;
SDL_Texture* spritesheet;
SDL_Window* window;
lua_State* L;
SDL_Texture* render_target;

int main(int argc, char* argv[]) {

    window = SDL_CreateWindow("SDL Random Colors with Image Overlay", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 512, 512, SDL_WINDOW_RESIZABLE);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
    spritesheet = IMG_LoadTexture(renderer, "assets/Untitled.png");
    render_target = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, SCREEN_WIDTH, SCREEN_HEIGHT);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    // system initialization
    audio_init();
    memory_init();

    // set up lua VM
    L = luaL_newstate();
    luaL_openlibs(L);

    // set up lua variables and functions
    lua_setup_draw();
    lua_setup_audio();
    lua_setup_functions();
    lua_setup_input();

    // load lua file
    luaL_dofile(L, "assets/script.lua");

    // check if special functions are set
    lua_getglobal(L, "_draw");
    bool draw_function_set = lua_isfunction(L, -1);
    lua_getglobal(L, "_music");
    bool music_function_set = lua_isfunction(L, -1);

    srand((unsigned int)time(NULL));
    bool running = true;
    int music_timer = 0;
    int frame_timer = 0;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        // execute draw function
        if (draw_function_set && (millis() - frame_timer > (1000 / 60))) {
            frame_timer = millis();

            // set and clear intermetiate render target
            SDL_SetRenderTarget(renderer, render_target);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
            SDL_RenderClear(renderer);

            lua_getglobal(L, "_draw");
            if (lua_pcall(L, 0, 1, 0) == LUA_OK) {
                lua_pop(L, lua_gettop(L));
            } else {
                destroyApplication();
                return 1;
            } 

            // redraw window
            SDL_SetRenderTarget(renderer, NULL);
            SDL_RenderCopyEx(renderer, render_target, NULL, NULL, 0, NULL, SDL_FLIP_NONE);
            SDL_RenderPresent(renderer);
        }

        // execute audio function
        // function is invoced every 8th beat
        if (music_function_set && (millis() - music_timer) > (60000 / bpm) / 8) {
            music_timer = millis();
            lua_getglobal(L, "_music");
            if (lua_pcall(L, 0, 1, 0) == LUA_OK) {
                lua_pop(L, lua_gettop(L));
            } else {
                destroyApplication();
                return 1;
            }
        }
        
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
    SDL_CloseAudioDevice(audio_device);
    SDL_Quit();
}
