
#ifdef _WIN32
    #include <SDL.h>
    #include <SDL_image.h>
#else
    #include <SDL2/SDL.h>
    #include <SDL2/SDL_image.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "tinybit/tinybit.h"
#include "cartridge_io.h"

void load_game(char* path) {
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

static void surface_to_buffer(SDL_Surface* surface, uint8_t* buffer) {
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

static void buffer_to_surface(uint8_t* buffer, SDL_Surface* surface) {
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

static char* read_file_to_string(const char* path) {
    FILE* fp = fopen(path, "r");
    if (!fp) {
        printf("Failed to open file: %s\n", path);
        exit(EXIT_FAILURE);
    }

    if (fseek(fp, 0L, SEEK_END) != 0) {
        printf("Failed to seek file: %s\n", path);
        fclose(fp);
        exit(EXIT_FAILURE);
    }

    long bufsize = ftell(fp);
    if (bufsize == -1) {
        printf("Failed to get file size: %s\n", path);
        fclose(fp);
        exit(EXIT_FAILURE);
    }

    char* source = malloc(sizeof(char) * (bufsize + 1));
    if (fseek(fp, 0L, SEEK_SET) != 0) {
        printf("Failed to rewind file: %s\n", path);
        free(source);
        fclose(fp);
        exit(EXIT_FAILURE);
    }

    size_t len = fread(source, sizeof(char), bufsize, fp);
    if (ferror(fp) != 0) {
        printf("Error reading file: %s\n", path);
        free(source);
        fclose(fp);
        exit(EXIT_FAILURE);
    }
    source[len] = '\0';

    fclose(fp);
    return source;
}

static SDL_Surface* load_image(const char* path) {
    SDL_Surface* surface = IMG_Load(path);
    if (!surface) {
        printf("Failed to load image '%s': %s\n", path, IMG_GetError());
        exit(EXIT_FAILURE);
    }
    return surface;
}

static void encode_spritesheet(uint8_t* dest, const uint8_t* src, long size, long* dest_index) {
    for (long i = 0; i < size; i++) {
        uint8_t byte = src[i];
        uint8_t a = (byte >> 6) & 0x3;
        uint8_t b = (byte >> 4) & 0x3;

        dest[*dest_index] = (dest[*dest_index] & 0xfc) | a;
        dest[*dest_index + 1] = (dest[*dest_index + 1] & 0xfc) | b;

        *dest_index += 2;
    }
}

static void encode_source(uint8_t* dest, const char* src, long size, long* dest_index) {
    for (long i = 0; i <= size; i++) {
        uint8_t byte = src[i];
        uint8_t a = (byte >> 6) & 0x3;
        uint8_t b = (byte >> 4) & 0x3;
        uint8_t c = (byte >> 2) & 0x3;
        uint8_t d = (byte >> 0) & 0x3;

        dest[*dest_index] = (dest[*dest_index] & 0xfc) | a;
        dest[*dest_index + 1] = (dest[*dest_index + 1] & 0xfc) | b;
        dest[*dest_index + 2] = (dest[*dest_index + 2] & 0xfc) | c;
        dest[*dest_index + 3] = (dest[*dest_index + 3] & 0xfc) | d;

        *dest_index += 4;
    }
}

void export_cartridge(char* sprite, char* script, char* cover, char* path) {
    char* source = read_file_to_string(script);

    int script_size = strlen(source);
    int cartridge_size = TB_MEM_SCRIPT_SIZE;

    if (script_size > cartridge_size) {
        printf("catridge too small to fit game\n");
        free(source);
        exit(EXIT_FAILURE);
    }

    printf("script size: %d\ncartridge size: %d\npercentage used: %d%%\n",
        script_size, cartridge_size,
        (int)((float)script_size / (float)cartridge_size * 100.0));

    uint8_t* buffer = (uint8_t*)malloc(TB_CARTRIDGE_WIDTH * TB_CARTRIDGE_HEIGHT * 4);
    uint8_t* spritebuffer = (uint8_t*)malloc(TB_SCREEN_WIDTH * TB_SCREEN_HEIGHT * 4);
    uint8_t* coverbuffer = (uint8_t*)malloc(TB_SCREEN_WIDTH * TB_SCREEN_HEIGHT * 4);

    SDL_Surface* cartridge = load_image("assets/cartridge3.png");
    surface_to_buffer(cartridge, buffer);

    SDL_Surface* spritesheet = load_image(sprite);
    surface_to_buffer(spritesheet, spritebuffer);

    SDL_Surface* coversurface = load_image(cover);
    surface_to_buffer(coversurface, coverbuffer);

    // embed cover image into cartridge buffer
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

    long dest_index = 0;
    encode_spritesheet(buffer, spritebuffer, TB_SCREEN_WIDTH * TB_SCREEN_HEIGHT * 4, &dest_index);
    encode_source(buffer, source, strlen(source), &dest_index);

    SDL_Surface* surface = SDL_CreateRGBSurface(0, TB_CARTRIDGE_WIDTH, TB_CARTRIDGE_HEIGHT, 32, 0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff);
    SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_BLEND);
    buffer_to_surface(buffer, surface);

    IMG_SavePNG(surface, path);

    free(source);
    free(buffer);
    free(spritebuffer);
    free(coverbuffer);
    SDL_FreeSurface(cartridge);
    SDL_FreeSurface(surface);
    SDL_FreeSurface(spritesheet);
    SDL_FreeSurface(coversurface);
    IMG_Quit();
    SDL_Quit();

    printf("Game exported\n");
}
