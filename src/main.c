
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
#ifdef _WIN32
    #include <windows.h>
#else
    #include <dirent.h>
#endif

#include "tinybit/tinybit.h"

struct TinyBitMemory tb_mem = {0};
uint8_t bs = 0;

SDL_Renderer* renderer;
SDL_Window* window;
SDL_Texture* render_target;

void play_game();
void export_cartridge(char*,char*,char*,char*);
void load_game(char*);

void print_usage() {
    printf("Usage: tinybit [] [-c spritesheet.png script.lua cover.png file.tb.png] [file.tb.png]\n");
    printf("empty => start game selector\n");
    printf("-c => compress a spritesheet and lua file into a cartridge file\n");
    printf("file => play a cartridge file\n");
}

void load_game(char* path) {
    // feed cartridge file
    FILE* fp = fopen(path, "rb");
    if (!fp) {
        printf("Failed to open cartridge file: %s\n", path);
        exit(EXIT_FAILURE);
    }
    uint8_t buffer[256];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
        if (!tinybit_feed_cartridge(buffer, bytes_read)) {
            printf("Error reading cartridge file: %s\n", path);
            fclose(fp);
            exit(EXIT_FAILURE);
        }
    }
    fclose(fp);
}


int game_count_cb() {
    int count = 0;
#ifdef _WIN32
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFileA("games\\*.tb.png", &findFileData);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            count++;
        } while (FindNextFileA(hFind, &findFileData) != 0);
        FindClose(hFind);
    }
#else
    DIR *dir = opendir("./games/");
    if (dir == NULL) {
        perror("opendir");
        return 0;
    }
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        const char *name = entry->d_name;
        size_t len = strlen(name);
        if (len >= 7 && strcmp(name + len - 7, ".tb.png") == 0) {
            count++;
        }
    }
    closedir(dir);
#endif
    return count;
}

void game_load_cb(int index){

    char filepath[256] = {0}; // Initialize to empty string
    int count = 0;

#ifdef _WIN32
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFileA("games\\*.tb.png", &findFileData);
    if (hFind == INVALID_HANDLE_VALUE) {
        return;
    }
    
    bool found = false;
    do {
        if (count == index) {
            snprintf(filepath, sizeof(filepath), "games\\%s", findFileData.cFileName);
            found = true;
            break;
        }
        count++;
    } while (FindNextFileA(hFind, &findFileData) != 0);
    FindClose(hFind);
    
    if (!found) {
        return;
    }
#else
    // open directory
    DIR *dir = opendir("./games/");
    if (dir == NULL) {
        perror("opendir");
        return;
    }

    // find the file at the given index
    struct dirent *entry;
    bool found = false;
    while ((entry = readdir(dir)) != NULL) {
        const char *name = entry->d_name;
        size_t len = strlen(name);
        if (len >= 7 && strcmp(name + len - 7, ".tb.png") == 0) {
            if (count == index) {
                snprintf(filepath, sizeof(filepath), "./games/%s", name);
                found = true;
                break;
            }
            count++;
        }
    }
    closedir(dir);
    
    if (!found) {
        printf("Game index %d not found\n", index);
        return;
    }
#endif

    // load file
    load_game(filepath);
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

    // expose functions to load games
    tinybit_log_cb(printf);
    tinybit_gamecount_cb(game_count_cb);
    tinybit_gameload_cb(game_load_cb);    

    // start game selector ui
    if(argc == 1) {
        tinybit_init(&tb_mem, &bs);
        tinybit_start();

        play_game();
    }

    // load game from file
    else if (argc == 2) {
        tinybit_init(&tb_mem, &bs);

        printf("Loading game: %s\n", argv[1]);

        // load game into memory
        load_game(argv[1]); 
        tinybit_start();

        play_game();
    }

    // export game to cartridge file
    else if (strcmp(argv[1], "-c") == 0 && argc == 6) {
        export_cartridge(argv[2],argv[3],argv[4],argv[5]);
    }

    // show help message
    else {
        print_usage();
        exit(EXIT_FAILURE);
    }

    return 0;
}

void export_cartridge(char* sprite, char* script, char* cover, char* path) {

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

    int script_size = strlen(source);
    int cartridge_size = TB_MEM_SCRIPT_SIZE;

    // check if game would fit in cartridge
    if (script_size > cartridge_size) {
        printf("catridge too small to fit game\n");
        exit(EXIT_FAILURE);
    }

    printf("script size: %d\ncartridge size: %d\npercentage used: %d%%\n", script_size, cartridge_size, (int)((float)script_size / (float)cartridge_size * 100.0));

    // allocate image buffer
    uint8_t* buffer = (uint8_t*)malloc(TB_CARTRIDGE_WIDTH * TB_CARTRIDGE_HEIGHT * 4);
    uint8_t* spritebuffer = (uint8_t*)malloc(TB_SCREEN_WIDTH * TB_SCREEN_HEIGHT * 4);
    uint8_t* coverbuffer = (uint8_t*)malloc(TB_SCREEN_WIDTH * TB_SCREEN_HEIGHT * 4);

    // fill buffer with cartridge image
    SDL_Surface* cartridge = IMG_Load("assets/cartridge2.png");
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

    SDL_Surface* coversurface = IMG_Load(cover);
    if (!coversurface) {
        printf("%s\n", IMG_GetError());
        exit(EXIT_FAILURE);
    }
    surface_to_buffer(coversurface, coverbuffer);

    // add cover image to cartridge buffer
    for (int y = 0; y < TB_SCREEN_HEIGHT; ++y) {
        for (int x = 0; x < TB_SCREEN_WIDTH; ++x) {
            uint32_t* target_pixel = (uint32_t*)(buffer + ((y+TB_COVER_Y) * TB_CARTRIDGE_WIDTH + (x+TB_COVER_X)) * 4);
            uint8_t r = coverbuffer[(y * TB_SCREEN_WIDTH + x) * 4];
            uint8_t g = coverbuffer[(y * TB_SCREEN_WIDTH + x) * 4 + 1];
            uint8_t b = coverbuffer[(y * TB_SCREEN_WIDTH + x) * 4 + 2];
            uint8_t a = coverbuffer[(y * TB_SCREEN_WIDTH + x) * 4 + 3];
            *target_pixel = a << 24 | b << 16 | g << 8 | r;
        }
    }

    long spritesheet_index = 0; // indicates pixel location in spritesheet buffer
    long source_index = 0; // indicates pixel location in source code buffer
    long dest_index = 0; // indicates pixel location in destination buffer

    // add spritesheet to cartridge buffer
    while(spritesheet_index < TB_SCREEN_WIDTH * TB_SCREEN_HEIGHT * 4) {
        // grab byte from spritesheet buffer
        uint8_t byte = spritebuffer[spritesheet_index];
        
        // split into 4 chunks of 2 bits each
        uint8_t a = (byte >> 6) & 0x3; // first 2 bits
        uint8_t b = (byte >> 4) & 0x3; // second 2 bits

        // write to destination buffer
        buffer[dest_index] = (buffer[dest_index] & 0xfc) | a; // first 2 bits
        buffer[dest_index + 1] = (buffer[dest_index + 1] & 0xfc) | b; // second 2 bits

        // increment indices
        spritesheet_index++;
        dest_index += 2;
    }

    // add source code to cartridge buffer
    while(source_index <= strlen(source)) {
        // grab byte from spritesheet buffer
        uint8_t byte = source[source_index];
        
        // split into 4 chunks of 2 bits each
        uint8_t a = (byte >> 6) & 0x3; // first 2 bits
        uint8_t b = (byte >> 4) & 0x3; // second 2 bits
        uint8_t c = (byte >> 2) & 0x3; // third 2 bits
        uint8_t d = (byte >> 0) & 0x3; // last 2 bits

        // write to destination buffer
        buffer[dest_index] = (buffer[dest_index] & 0xfc) | a; // first 2 bits
        buffer[dest_index + 1] = (buffer[dest_index + 1] & 0xfc) | b; // second 2 bits
        buffer[dest_index + 2] = (buffer[dest_index + 2] & 0xfc) | c; // third 2 bits
        buffer[dest_index + 3] = (buffer[dest_index + 3] & 0xfc) | d; // fourth 2 bits

        // increment indices
        source_index++;
        dest_index += 4;
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
    SDL_FreeSurface(spritesheet);
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
    SDL_Event event;

    while (running) {
        uint32_t frame_start = SDL_GetTicks();

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        // execute draw function
        // (no need to check millis/frame_timer here, just run every loop)
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

        // quit if ESC is pressed
        if(state[(SDL_Scancode) SDL_SCANCODE_ESCAPE] == 1) {
            return;
        }

        if (!tinybit_frame()) {
            printf("script failed");
        }

        // map display section to render target
        uint32_t* pixels;
        int pitch;
        SDL_LockTexture(render_target, NULL, (void**)&pixels, &pitch);

        for (int y = 0; y < 128; ++y) {
            for (int x = 0; x < 128; ++x) {
                uint8_t r = (tinybit_memory->display[(y * TB_SCREEN_WIDTH + x) * 2 + 0] << 0) & 0xf0;
                uint8_t g = (tinybit_memory->display[(y * TB_SCREEN_WIDTH + x) * 2 + 0] << 4) & 0xf0;
                uint8_t b = (tinybit_memory->display[(y * TB_SCREEN_WIDTH + x) * 2 + 1] << 0) & 0xf0;
                uint8_t a = (tinybit_memory->display[(y * TB_SCREEN_WIDTH + x) * 2 + 1] << 4) & 0xf0;

                pixels[y * (pitch / 4) + x] = r << 24 | g << 16 | b << 8 | a;
            }
        }

        SDL_UnlockTexture(render_target);

        // redraw window
        SDL_SetRenderTarget(renderer, NULL);
        SDL_RenderCopy(renderer, render_target, NULL, NULL);
        SDL_RenderPresent(renderer);

        // --- FPS limiting ---
        uint32_t frame_time = SDL_GetTicks() - frame_start;
        if (frame_time < 16) {
            SDL_Delay(16 - frame_time);
        }
    }

    SDL_DestroyTexture(render_target);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    //SDL_CloseAudioDevice(audio_device);
    SDL_Quit();
}