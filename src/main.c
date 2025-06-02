
#ifdef _WIN32
    #include <SDL.h>
    #include <SDL_image.h>
#else
    #include <SDL2/SDL.h>
    #include <SDL2/SDL_image.h>
#endif

#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

#include "tinybit/tinybit.h"

struct TinyBitMemory tb_mem = {0};
uint8_t bs = 0;

SDL_Renderer* renderer;
SDL_Window* window;
SDL_Texture* render_target;

void play_game();
void export_cartridge(char*,char*,char*);
void load_game(char*);

void print_usage() {
    printf("Usage: program [-c file.png file.lua file.tb.png] [-p file.png file.lua] [file.tb.png]\n");
    printf("-c => compress a spritesheet and lua file into a cartridge file\n");
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
    if (strcmp(command, "-c") == 0 && argc == 5) {
        export_cartridge(argv[2],argv[3],argv[4]);
    }
    else if (argc == 2) {
        load_game(argv[1]);
    }
    else {
        print_usage();
        exit(EXIT_FAILURE);
    }

    return 0;
}

void load_game(char* path) {
    tinybit_init(&tb_mem, &bs);
    FILE *fp = fopen(path, "rb");

    // Read the PNG file in chunks
    uint8_t buf[1024];
    size_t len;
    while ((len = fread(buf, 1, sizeof(buf), fp)) > 0) {
        tinybit_feed_catridge(buf, len);
    }

    fclose(fp);
    tinybit_start();
    play_game();
}

void export_cartridge(char* sprite, char* script, char* path) {

    // load lua file
    char* source = NULL;
    FILE* fp = fopen(script, "r");
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

    int game_size = TB_SCREEN_WIDTH * TB_SCREEN_HEIGHT * 4 + strlen(source);
    int cartridge_size = TB_CARTRIDGE_WIDTH * TB_CARTRIDGE_HEIGHT;

    // check if game would fit in cartridge
    if (game_size > cartridge_size) {
        printf("catridge too small to fit game\n");
        exit(EXIT_FAILURE);
    }

    printf("game size: %d\ncartridge size: %d\npercentage used: %d%%\n", game_size, cartridge_size, (int)((float)game_size / (float)cartridge_size * 100.0));

    // allocate image buffer
    uint8_t* buffer = (uint8_t*)malloc(TB_CARTRIDGE_WIDTH * TB_CARTRIDGE_HEIGHT * 4);
    uint8_t* spritebuffer = (uint8_t*)malloc(TB_SCREEN_WIDTH * TB_SCREEN_HEIGHT * 4);

    // fill buffer with cartridge image
    SDL_Surface* cartridge = IMG_Load("assets/cartridge.png");
    if (!cartridge) {
        printf("%s\n", IMG_GetError());
        exit(EXIT_FAILURE);
    }
    surface_to_buffer(cartridge, buffer);

    // fill buffer with spritesheet image
    SDL_Surface* spritesheet = IMG_Load(sprite);
    if (!spritesheet) {
        printf("%s\n", IMG_GetError());
        exit(EXIT_FAILURE);
    }
    surface_to_buffer(spritesheet, spritebuffer);


    // add cover, spritesheet and game data
    for (int y = 0; y < TB_CARTRIDGE_HEIGHT; y++) {
        for (int x = 0; x < TB_CARTRIDGE_WIDTH; x++) {

            long pixel_index = ((y * TB_CARTRIDGE_WIDTH) + x) * 4; // indicates pixel location in buffer
            long byte_index = ((y * TB_CARTRIDGE_WIDTH) + x); // indicates which byte we are saving

            uint8_t* r = &buffer[pixel_index];
            uint8_t* g = &buffer[pixel_index + 1];
            uint8_t* b = &buffer[pixel_index + 2];
            uint8_t* a = &buffer[pixel_index + 3];

            // spritesheet data
            if (byte_index < TB_SCREEN_WIDTH * TB_SCREEN_HEIGHT * 4) {
                *r = (*r & 0xfc) | ((spritebuffer[byte_index] >> 6) & 0x3);
                *g = (*g & 0xfc) | ((spritebuffer[byte_index] >> 4) & 0x3);
                *b = (*b & 0xfc) | ((spritebuffer[byte_index] >> 2) & 0x3);
                *a = (*a & 0xfc) | ((spritebuffer[byte_index] >> 0) & 0x3);
            }

            // source code
            else if (byte_index - TB_SCREEN_WIDTH * TB_SCREEN_HEIGHT * 4 < game_size) {
                *r = (*r & 0xfc) | ((source[byte_index - TB_SCREEN_HEIGHT * TB_SCREEN_WIDTH * 4] >> 6) & 0x3);
                *g = (*g & 0xfc) | ((source[byte_index - TB_SCREEN_HEIGHT * TB_SCREEN_WIDTH * 4] >> 4) & 0x3);
                *b = (*b & 0xfc) | ((source[byte_index - TB_SCREEN_HEIGHT * TB_SCREEN_WIDTH * 4] >> 2) & 0x3);
                *a = (*a & 0xfc) | ((source[byte_index - TB_SCREEN_HEIGHT * TB_SCREEN_WIDTH * 4] >> 0) & 0x3);
            }

        }
    }

    // save buffer to surface
    SDL_Surface* surface = SDL_CreateRGBSurface(0, TB_CARTRIDGE_WIDTH, TB_CARTRIDGE_HEIGHT, 32, 0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff);
    SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_BLEND);

    buffer_to_surface(buffer, surface);

    free(source);

    // save cartridge
    IMG_SavePNG(surface, path);

    SDL_FreeSurface(cartridge);
    SDL_FreeSurface(surface);
    SDL_FreeSurface(sprite);
    IMG_Quit();
    SDL_Quit();

    printf("Game exported\n");

    return;
}

void play_game() {
    window = SDL_CreateWindow("TinyBit Virtual Console", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 512, 512, SDL_WINDOW_RESIZABLE);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
    render_target = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 128, 128);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetTextureBlendMode(render_target, SDL_BLENDMODE_BLEND);
    
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
        if ((millis() - frame_timer > (1000 / 60))) {
            frame_timer = millis();

            // set and clear intermediate render target
            SDL_SetRenderTarget(renderer, render_target);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
            SDL_RenderClear(renderer);

            // get button input
            const char* state = (const char*)SDL_GetKeyboardState(NULL);
            if(state[(SDL_Scancode) SDL_SCANCODE_UP] == 1) {
                bs |= (1 << TB_BUTTON_UP);
            } else {
                bs &= ~(1 << TB_BUTTON_UP);
            }
            if(state[(SDL_Scancode) SDL_SCANCODE_DOWN] == 1) {
                bs |= (1 << TB_BUTTON_DOWN);
            } else {
                bs &= ~(1 << TB_BUTTON_DOWN);
            }
            if(state[(SDL_Scancode) SDL_SCANCODE_LEFT] == 1) {
                bs |= (1 << TB_BUTTON_LEFT);
            } else {
                bs &= ~(1 << TB_BUTTON_LEFT);
            }
            if(state[(SDL_Scancode) SDL_SCANCODE_RIGHT] == 1) {
                bs |= (1 << TB_BUTTON_RIGHT);
            } else {
                bs &= ~(1 << TB_BUTTON_RIGHT);
            }
            if(state[(SDL_Scancode) SDL_SCANCODE_A] == 1) {
                bs |= (1 << TB_BUTTON_A);
            } else {
                bs &= ~(1 << TB_BUTTON_A);
            }
            if(state[(SDL_Scancode) SDL_SCANCODE_B] == 1) {
                bs |= (1 << TB_BUTTON_B);
            } else {
                bs &= ~(1 << TB_BUTTON_B);
            }

            tinybit_frame();

            // map display section to render target
            uint32_t* pixels;
            int pitch;
            SDL_LockTexture(render_target, NULL, (void**)&pixels, &pitch);

            for (int y = 0; y < 128; ++y) {
                for (int x = 0; x < 128; ++x) {
                    uint8_t r = tinybit_memory->display[(y * TB_SCREEN_WIDTH + x) * 4];
                    uint8_t g = tinybit_memory->display[(y * TB_SCREEN_WIDTH + x) * 4 + 1];
                    uint8_t b = tinybit_memory->display[(y * TB_SCREEN_WIDTH + x) * 4 + 2];
                    uint8_t a = tinybit_memory->display[(y * TB_SCREEN_WIDTH + x) * 4 + 3];

                    pixels[y * (pitch / 4) + x] = r << 24 | g << 16 | b << 8 | a;
                }
            }

            SDL_UnlockTexture(render_target);

            // redraw window
            SDL_SetRenderTarget(renderer, NULL);
            SDL_RenderCopy(renderer, render_target, NULL, NULL);
            SDL_RenderPresent(renderer);
        }


    }

    SDL_DestroyTexture(render_target);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    //SDL_CloseAudioDevice(audio_device);
    SDL_Quit();
}