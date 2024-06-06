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
SDL_Window* window;
lua_State* L;
SDL_Texture* render_target;

int main(int argc, char* argv[]) {

    window = SDL_CreateWindow("SDL Random Colors with Image Overlay", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 512, 512, SDL_WINDOW_RESIZABLE);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
    render_target = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 128, 128);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetTextureBlendMode(render_target, SDL_BLENDMODE_BLEND);

    // system initialization
    audio_init();
    memory_init();

    // load spritesheet
    SDL_Surface* image = IMG_Load("assets/flappy.png");
    if (!image) {
        printf("IMG_Load: %s\n", IMG_GetError());
        return 1;
    }

    SDL_LockSurface(image);
    for (int i = 0; i < 128 * 128; i++) {
        uint32_t* pixels = (uint32_t*)image->pixels;
        SDL_GetRGBA(
            pixels[i],
            image->format,
            &memory[MEM_SPRITESHEET_START + i * 4],
            &memory[MEM_SPRITESHEET_START + i * 4 + 1],
            &memory[MEM_SPRITESHEET_START + i * 4 + 2],
            &memory[MEM_SPRITESHEET_START + i * 4 + 3]
        );
    }
    SDL_UnlockSurface(image);
    SDL_FreeSurface(image);

    // copy to display buffer for now
    memcpy(memory + MEM_DISPLAY_START, memory + MEM_SPRITESHEET_START, MEM_SPRITESHEET_SIZE);

    // set up lua VM
    L = luaL_newstate();
    luaL_openlibs(L);

    // set up lua variables and functions
    lua_setup_draw();
    lua_setup_audio();
    lua_setup_functions();
    lua_setup_input();
    lua_setup_memory();

    // load lua file
    luaL_dofile(L, "assets/flappy.lua");

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

        // execute draw function, limit to 60fps
        if (draw_function_set && (millis() - frame_timer > (1000 / 60))) {
            frame_timer = millis();

            // set and clear intermediate render target
            SDL_SetRenderTarget(renderer, render_target);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
            SDL_RenderClear(renderer);

            // perform lua draw function every frame
            lua_getglobal(L, "_draw");
            if (lua_pcall(L, 0, 1, 0) == LUA_OK) {
                lua_pop(L, lua_gettop(L));
            } else {
                destroyApplication();
                return 1;
            } 

            // map display section to render target
            uint32_t* pixels;
            int pitch;
            SDL_LockTexture(render_target, NULL, &pixels, &pitch);

            for (int y = 0; y < 128; ++y) {
                for (int x = 0; x < 128; ++x) {
                    uint8_t r = memory[MEM_DISPLAY_START + (y * SCREEN_WIDTH + x) * 4];
                    uint8_t g = memory[MEM_DISPLAY_START + (y * SCREEN_WIDTH + x) * 4 + 1];
                    uint8_t b = memory[MEM_DISPLAY_START + (y * SCREEN_WIDTH + x) * 4 + 2];
                    uint8_t a = memory[MEM_DISPLAY_START + (y * SCREEN_WIDTH + x) * 4 + 3];

                    pixels[y * (pitch / 4) + x] = r << 24 | g << 16 | b << 8 | a;
                }
            }

            SDL_UnlockTexture(render_target);

            // redraw window
            SDL_SetRenderTarget(renderer, NULL);
            SDL_RenderCopy(renderer, render_target, NULL, NULL);
            SDL_RenderPresent(renderer);
        }

        // execute audio function
        // function is invoked every 8th beat
        if (music_function_set && (millis() - music_timer) > (60000 / bpm) / 8) {
            music_timer = millis();
            lua_getglobal(L, "_music");
            if (lua_pcall(L, 0, 1, 0) == LUA_OK) {
                lua_pop(L, lua_gettop(L));
            }
            else {
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
    SDL_DestroyTexture(render_target);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_CloseAudioDevice(audio_device);
    SDL_Quit();
}
