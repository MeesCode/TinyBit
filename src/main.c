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
#include "audio_functions.h"


// Screen dimensions
const int SCREEN_WIDTH = 128;
const int SCREEN_HEIGHT = 128;

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

    // set up lua VM and functions
    L = luaL_newstate();
    luaL_openlibs(L);


    // set lua screen variables 
    lua_pushinteger(L, SCREEN_WIDTH); 
    lua_setglobal(L, "SCREEN_WIDTH");
    lua_pushinteger(L, SCREEN_HEIGHT);
    lua_setglobal(L, "SCREEN_HEIGHT");

    // set lua tone variables
    lua_pushinteger(L, C);
    lua_setglobal(L, "C");
    lua_pushinteger(L, Cs);
    lua_setglobal(L, "Cs");
    lua_pushinteger(L, Db);
    lua_setglobal(L, "Db");
    lua_pushinteger(L, D);
    lua_setglobal(L, "D");
    lua_pushinteger(L, Ds);
    lua_setglobal(L, "Ds");
    lua_pushinteger(L, Eb);
    lua_setglobal(L, "Eb");
    lua_pushinteger(L, E);
    lua_setglobal(L, "E");
    lua_pushinteger(L, F);
    lua_setglobal(L, "F");
    lua_pushinteger(L, Fs);
    lua_setglobal(L, "Fs");
    lua_pushinteger(L, Gb);
    lua_setglobal(L, "Gb");
    lua_pushinteger(L, G);
    lua_setglobal(L, "G");
    lua_pushinteger(L, Gs);
    lua_setglobal(L, "Gs");
    lua_pushinteger(L, Ab);
    lua_setglobal(L, "Ab");
    lua_pushinteger(L, A);
    lua_setglobal(L, "A");
    lua_pushinteger(L, As);
    lua_setglobal(L, "As");
    lua_pushinteger(L, Bb);
    lua_setglobal(L, "Bb");
    lua_pushinteger(L, B);
    lua_setglobal(L, "B");

    // set lua waveforms
    lua_pushinteger(L, SIN);
    lua_setglobal(L, "SIN");


    // set lua functions
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
    lua_pushcfunction(L, lua_tone);
    lua_setglobal(L, "tone");
    lua_pushcfunction(L, lua_bpm);
    lua_setglobal(L, "bpm");

    // load lua file
    luaL_dofile(L, "assets/script.lua");

    lua_getglobal(L, "_draw");
    bool draw_function_set = lua_isfunction(L, -1);
    lua_getglobal(L, "_music");
    bool music_function_set = lua_isfunction(L, -1);

    // set up audio
    SDL_Init(SDL_INIT_AUDIO);
    SDL_zero(audio_spec);
    audio_spec.freq = 44100;
    audio_spec.format = AUDIO_S16SYS;
    audio_spec.channels = 1;
    audio_spec.samples = 1024;
    audio_spec.callback = NULL;
    audio_device = SDL_OpenAudioDevice(NULL, 0, &audio_spec, NULL, 0);
    SDL_PauseAudioDevice(audio_device, 0);

    srand((unsigned int)time(NULL));
    bool running = true;
    int music_timer = 0;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        // set and clear intermetiate render target
        SDL_SetRenderTarget(renderer, render_target);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderClear(renderer);

        // execute draw function
        if(draw_function_set){
            lua_getglobal(L, "_draw");
            if (lua_pcall(L, 0, 1, 0) == LUA_OK) {
                lua_pop(L, lua_gettop(L));
            }
            else {
                destroyApplication();
                return 1;
            }
        }

        // execute draw function
        if (draw_function_set) {
            lua_getglobal(L, "_draw");
            if (lua_pcall(L, 0, 1, 0) == LUA_OK) {
                lua_pop(L, lua_gettop(L));
            }
            else {
                destroyApplication();
                return 1;
            }
        }

        // execute audio function
        // function is invoced every 8th beat
        if (music_function_set && (millis() - music_timer) > (60000 / bpm) / 8) {
            lua_getglobal(L, "_music");
            if (lua_pcall(L, 0, 1, 0) == LUA_OK) {
                lua_pop(L, lua_gettop(L));
            }
            else {
                destroyApplication();
                return 1;
            }
        }
        
        // set window as render target
        SDL_SetRenderTarget(renderer, NULL);

        // redraw render target
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderClear(renderer);
        SDL_RenderCopyEx(renderer, render_target, NULL, NULL, 0, NULL, SDL_FLIP_NONE);
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
    SDL_CloseAudioDevice(audio_device);
    SDL_Quit();
}
