
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
void boot_console(SDL_Surface*, char*);
void export_cartridge(SDL_Surface*, char*, char*);
void open_cartridge();

void print_usage() {
    printf("Usage: program [-c file.png file.lua file.tb.png] [-p file.png file.lua] [file.tb.png]\n");
    printf("-c => compress a spritesheet and lua file into a cartridge file\n");
    printf("-p => play a spritesheet and lua file\n");
    printf("no flags => play a cartridge file\n");
}

void surface_to_buffer(SDL_Surface* surface, uint8_t* buffer) {
    SDL_LockSurface(surface);
    for (int i = 0; i < surface->w * surface->h; i++) {
        uint32_t* pixels = (uint32_t*)surface->pixels;
        SDL_GetRGBA(
            pixels[i],
            surface->format,
            buffer + i * 4,
            buffer + i * 4 + 1,
            buffer + i * 4 + 2,
            buffer + i * 4 + 3
        );
    }
    SDL_UnlockSurface(surface);
}

void buffer_to_surface(uint8_t* buffer, SDL_Surface* surface) {
    SDL_LockSurface(surface);
    for (int y = 0; y < surface->h; ++y) {
        for (int x = 0; x < surface->w; ++x) {
            uint32_t* target_pixel = (uint32_t*)((uint8_t*)surface->pixels + y * surface->pitch + x * surface->format->BytesPerPixel);
            uint8_t r = buffer[(y * surface->w + x) * 4];
            uint8_t g = buffer[(y * surface->w + x) * 4 + 1];
            uint8_t b = buffer[(y * surface->w + x) * 4 + 2];
            uint8_t a = buffer[(y * surface->w + x) * 4 + 3];
            *target_pixel = r << 24 | g << 16 | b << 8 | a;
        }
    }
    SDL_UnlockSurface(surface);
}

int main(int argc, char* argv[]) {

    if (argc < 2) {
        print_usage();
        exit(EXIT_FAILURE);
    }

    char* command = argv[1];
    if ((strcmp(command, "-c") == 0 && argc == 5) || (strcmp(command, "-p") == 0 && argc == 4)) {

        // load spritesheet
        SDL_Surface* image = IMG_Load(argv[2]);
        if (!image) {
            printf("IMG_Load: %s\n", IMG_GetError());
            exit(EXIT_FAILURE);
        }

        // load lua file
        char* source = NULL;
        FILE* fp = fopen(argv[3], "r");
        if (fp != NULL) {
            if (fseek(fp, 0L, SEEK_END) == 0) {
                long bufsize = ftell(fp);
                if (bufsize == -1) { /* Error */ }

                source = malloc(sizeof(char) * (bufsize + 1));

                if (fseek(fp, 0L, SEEK_SET) != 0) { /* Error */ }

                size_t newLen = fread(source, sizeof(char), bufsize, fp);
                if (ferror(fp) != 0) {
                    printf("Error reading file");
                    exit(EXIT_FAILURE);
                }
                else {
                    source[newLen++] = '\0'; /* Just to be safe. */
                }
            }
            fclose(fp);
        }

        if (strcmp(command, "-c") == 0) {
            export_cartridge(image, source, argv[4]);
        }
        else {
            play_game(image, source);
        }

        free(source);
        SDL_FreeSurface(image);

    }
    else if (argc == 2) {
        open_cartridge(argv[1]);
    }
    else {
        print_usage();
        exit(EXIT_FAILURE);
    }

    return 0;
}

void printb(uint8_t v) {
    unsigned int i, s = 1 << ((sizeof(v) << 3) - 1); // s = only most significant bit at 1
    for (i = s; i; i >>= 1) printf("%d", v & i || 0);
}

void open_cartridge(char* path) {
    
    SDL_Surface* cartridge = IMG_Load(path);
    if (!cartridge) {
        printf("IMG_Load: %s\n", IMG_GetError());
        exit(EXIT_FAILURE);
    }

    uint8_t* cartridge_buffer = (uint8_t*)malloc(CARTRIDGE_WIDTH * CARTRIDGE_HEIGHT * 4);
    uint8_t* spritesheet_buffer = (uint8_t*)malloc(SCREEN_WIDTH * SCREEN_HEIGHT * 4);
    char* source_buffer = (char*)malloc((CARTRIDGE_WIDTH * CARTRIDGE_HEIGHT * 4) - (SCREEN_WIDTH * SCREEN_HEIGHT * 4));

    surface_to_buffer(cartridge, cartridge_buffer);

    // iterate over cartridge buffer and extract spritesheet and source code
    for (int y = 0; y < CARTRIDGE_HEIGHT; y++) {
        for (int x = 0; x < CARTRIDGE_WIDTH; x++) {

            long pixel_index = ((y * CARTRIDGE_WIDTH) + x) * 4; // indicates pixel location in buffer
            long byte_index = ((y * CARTRIDGE_WIDTH) + x); // indicates which byte we are saving

            uint8_t r = cartridge_buffer[pixel_index];
            uint8_t g = cartridge_buffer[pixel_index + 1];
            uint8_t b = cartridge_buffer[pixel_index + 2];
            uint8_t a = cartridge_buffer[pixel_index + 3];

            // spritesheet data
            if (byte_index < SCREEN_WIDTH * SCREEN_HEIGHT * 4) {
                spritesheet_buffer[byte_index] = (r & 0x3) << 6 | (g & 0x3) << 4 | (b & 0x3) << 2 | (a & 0x3) << 0;
            }

            // source code
            else if (byte_index - SCREEN_WIDTH * SCREEN_HEIGHT * 4 < CARTRIDGE_WIDTH * CARTRIDGE_HEIGHT * 4) {
                source_buffer[byte_index - SCREEN_HEIGHT * SCREEN_WIDTH * 4] = (r & 0x3) << 6 | (g & 0x3) << 4 | (b & 0x3) << 2 | (a & 0x3) << 0;
            }
        }
    }

    SDL_Surface* surface = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32, 0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff);
    SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_BLEND);
    
    buffer_to_surface(spritesheet_buffer, surface);
    
    play_game(surface, source_buffer);
}

void export_cartridge(SDL_Surface* image, char* source, char* path) {

    int game_size = SCREEN_WIDTH * SCREEN_HEIGHT * 4 + strlen(source);
    int cartridge_size = CARTRIDGE_WIDTH * CARTRIDGE_WIDTH;

    // check if game would fit in cartridge
    if (game_size > cartridge_size) {
        printf("catridge too small to fit game\n");
        exit(EXIT_FAILURE);
    }

    printf("game size: %d\ncartridge size: %d\npercentage used: %d%%\n", game_size, cartridge_size, (int)((float)game_size / (float)cartridge_size * 100.0));

    // allocate image buffer
    uint8_t* buffer = (uint8_t*)malloc(CARTRIDGE_WIDTH * CARTRIDGE_HEIGHT * 4);

    boot_console(image, source);

    // check for titlescreen function
    lua_getglobal(L, "_titlescreen");
    bool titlescreen_function = lua_isfunction(L, -1);

    if (!titlescreen_function) {
        printf("titlescreen function not available\n");
    }
    else {
        lua_getglobal(L, "_titlescreen");
        if (lua_pcall(L, 0, 1, 0) == LUA_OK) {
            lua_pop(L, lua_gettop(L));
        }
    }

    // TODO: possibly reload game, since spritesheet might be modified by _titlescreen funciton

    // fill buffer with cartridge image

    SDL_Surface* cartridge = IMG_Load("assets/cartridge.png");
    if (!cartridge) {
        printf("IMG_Load: %s\n", IMG_GetError());
        exit(EXIT_FAILURE);
    }

    surface_to_buffer(cartridge, buffer);

    // add cover, spritesheet and game data
    for (int y = 0; y < CARTRIDGE_HEIGHT; y++) {
        for (int x = 0; x < CARTRIDGE_WIDTH; x++) {

            long pixel_index = ((y * CARTRIDGE_WIDTH) + x) * 4; // indicates pixel location in buffer
            long byte_index = ((y * CARTRIDGE_WIDTH) + x); // indicates which byte we are saving

            uint8_t* r = &buffer[pixel_index];
            uint8_t* g = &buffer[pixel_index + 1];
            uint8_t* b = &buffer[pixel_index + 2];
            uint8_t* a = &buffer[pixel_index + 3];

            // cover image
            if (x >= 40 && x < 40 + 256 && y >= 40 && y < 40 + 256) {
                *r = memory[MEM_DISPLAY_START + (((y - 40) / 2) * SCREEN_WIDTH + ((x - 40) / 2)) * 4];
                *g = memory[MEM_DISPLAY_START + (((y - 40) / 2) * SCREEN_WIDTH + ((x - 40) / 2)) * 4 + 1];
                *b = memory[MEM_DISPLAY_START + (((y - 40) / 2) * SCREEN_WIDTH + ((x - 40) / 2)) * 4 + 2];
                *a = memory[MEM_DISPLAY_START + (((y - 40) / 2) * SCREEN_WIDTH + ((x - 40) / 2)) * 4 + 3];
            }

            // spritesheet data
            if (byte_index < SCREEN_WIDTH * SCREEN_HEIGHT * 4) {
                *r = (*r & 0xfc) | (memory[MEM_SPRITESHEET_START + byte_index] >> 6) & 0x3;
                *g = (*g & 0xfc) | (memory[MEM_SPRITESHEET_START + byte_index] >> 4) & 0x3;
                *b = (*b & 0xfc) | (memory[MEM_SPRITESHEET_START + byte_index] >> 2) & 0x3;
                *a = (*a & 0xfc) | (memory[MEM_SPRITESHEET_START + byte_index] >> 0) & 0x3;
            }

            // source code
            else if (byte_index - SCREEN_WIDTH * SCREEN_HEIGHT * 4 < game_size) {
                *r = (*r & 0xfc) | (source[byte_index - SCREEN_HEIGHT * SCREEN_WIDTH * 4] >> 6) & 0x3;
                *g = (*g & 0xfc) | (source[byte_index - SCREEN_HEIGHT * SCREEN_WIDTH * 4] >> 4) & 0x3;
                *b = (*b & 0xfc) | (source[byte_index - SCREEN_HEIGHT * SCREEN_WIDTH * 4] >> 2) & 0x3;
                *a = (*a & 0xfc) | (source[byte_index - SCREEN_HEIGHT * SCREEN_WIDTH * 4] >> 0) & 0x3;
            }

        }
    }

    // save buffer to surface
    SDL_Surface* surface = SDL_CreateRGBSurface(0, CARTRIDGE_WIDTH, CARTRIDGE_HEIGHT, 32, 0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff);
    SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_BLEND);

    buffer_to_surface(buffer, surface);

    // save cartridge
    IMG_SavePNG(surface, path);

    SDL_FreeSurface(cartridge);
    SDL_FreeSurface(surface);
    lua_close(L);
    IMG_Quit();
    SDL_Quit();

    printf("Game exported\n");

    return 0;
}

void play_game(SDL_Surface* image, char* source) {
    window = SDL_CreateWindow("TinyBit Virtual Console", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 512, 512, SDL_WINDOW_RESIZABLE);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
    render_target = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 128, 128);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetTextureBlendMode(render_target, SDL_BLENDMODE_BLEND);

    boot_console(image, source);

    // check if special functions are set
    lua_getglobal(L, "_draw");
    bool draw_function_set = lua_isfunction(L, -1);
    lua_getglobal(L, "_music");
    bool music_function_set = lua_isfunction(L, -1);
    
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
            } else {
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

void boot_console(SDL_Surface* image, char* source) {
    // system initialization
    audio_init();
    memory_init();

    surface_to_buffer(image, memory + MEM_SPRITESHEET_START);

    // load font
    SDL_Surface* font = IMG_Load("assets/font.png");
    if (!font) {
        printf("IMG_Load: %s\n", IMG_GetError());
        return 1;
    }

    surface_to_buffer(font, memory + MEM_FONT_START);

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
    srand((unsigned int)time(NULL));

    // load lua file
    if (luaL_dostring(L, source) == LUA_OK) {
        lua_pop(L, lua_gettop(L));
    }
}