
#ifdef _WIN32
    #include "getopt.h"
    #include <SDL.h>
    #include <SDL_image.h>
#else
    #include <unistd.h>
    #include <SDL/SDL.h>
    #include <SDL/SDL_image.h>
#endif


#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <stdio.h>
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

void play_game(SDL_Surface*, char*);

void print_usage() {
    printf("Usage: program [-c file1 file2] [-e file1 file2] [-p file1 file2] [file]\n");
}

int main(int argc, char* argv[]) {

    if (argc < 2) {
        print_usage();
        exit(EXIT_FAILURE);
    }

    char* command = argv[1];
    if (strcmp(command, "-c") == 0 || strcmp(command, "-e") == 0 || strcmp(command, "-p") == 0) {
        if (argc != 4) {
            print_usage();
            exit(EXIT_FAILURE);
        }
        char* file1 = argv[2];
        char* file2 = argv[3];
        printf("Command: %s, File1: %s, File2: %s\n", command, file1, file2);
        // Add your logic to handle the commands with two files here
    }
    else {
        // If the first argument is not -c, -e, or -p, treat it as a single file argument
        if (argc != 2) {
            print_usage();
            exit(EXIT_FAILURE);
        }
        char* file = argv[1];
        printf("Single file: %s\n", file);
        // Add your logic to handle the single file here
    }

    // load spritesheet
    SDL_Surface* image = IMG_Load(argv[3]);
    if (!image) {
        printf("IMG_Load: %s\n", IMG_GetError());
        return 1;
    }

    // load lua file
    char* source = NULL;
    FILE* fp = fopen(argv[2], "r");
    if (fp != NULL) {
        /* Go to the end of the file. */
        if (fseek(fp, 0L, SEEK_END) == 0) {
            /* Get the size of the file. */
            long bufsize = ftell(fp);
            if (bufsize == -1) { /* Error */ }

            /* Allocate our buffer to that size. */
            source = malloc(sizeof(char) * (bufsize + 1));

            /* Go back to the start of the file. */
            if (fseek(fp, 0L, SEEK_SET) != 0) { /* Error */ }

            /* Read the entire file into memory. */
            size_t newLen = fread(source, sizeof(char), bufsize, fp);
            if (ferror(fp) != 0) {
                fputs("Error reading file", stderr);
            }
            else {
                source[newLen++] = '\0'; /* Just to be safe. */
            }
        }
        fclose(fp);
    }

    play_game(image, source);
    free(source);
    SDL_FreeSurface(image);

    return 0;
}

void play_game(SDL_Surface* image, char* source) {
    window = SDL_CreateWindow("TinyBit Virtual Console", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 512, 512, SDL_WINDOW_RESIZABLE);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
    render_target = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 128, 128);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetTextureBlendMode(render_target, SDL_BLENDMODE_BLEND);

    // system initialization
    audio_init();
    memory_init();

    SDL_LockSurface(image);
    for (int i = 0; i < image->w * image->h; i++) {
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

    // load font
    image = IMG_Load("assets/font.png");
    if (!image) {
        printf("IMG_Load: %s\n", IMG_GetError());
        return 1;
    }

    SDL_LockSurface(image);
    for (int i = 0; i < image->w * image->h; i++) {
        uint32_t* pixels = (uint32_t*)image->pixels;
        SDL_GetRGBA(
            pixels[i],
            image->format,
            &memory[MEM_FONT_START + i * 4],
            &memory[MEM_FONT_START + i * 4 + 1],
            &memory[MEM_FONT_START + i * 4 + 2],
            &memory[MEM_FONT_START + i * 4 + 3]
        );
    }
    SDL_UnlockSurface(image);
    SDL_FreeSurface(image);

    // set up lua VM
    L = luaL_newstate();

    // load lua libraries
    static const luaL_Reg loadedlibs[] = {
        {LUA_GNAME, luaopen_base},
        {LUA_COLIBNAME, luaopen_coroutine},
        {LUA_TABLIBNAME, luaopen_table},
        {LUA_STRLIBNAME, luaopen_string},
        {LUA_MATHLIBNAME, luaopen_math},
        {NULL, NULL}
    };

    const luaL_Reg* lib;
    for (lib = loadedlibs; lib->func; lib++) {
        luaL_requiref(L, lib->name, lib->func, 1);
        lua_pop(L, 1);  /* remove lib */
    }

    // set up lua variables and functions
    lua_setup_draw();
    lua_setup_audio();
    lua_setup_functions();
    lua_setup_input();
    lua_setup_memory();

    // load lua file
    if (luaL_dostring(L, source) == LUA_OK) {
        lua_pop(L, lua_gettop(L));
    }

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
            }
            else {
                break;
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
                break;
            }
        }

    }

    lua_close(L);
    SDL_DestroyTexture(render_target);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_CloseAudioDevice(audio_device);
    SDL_Quit();
}