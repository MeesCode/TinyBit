
#include <SDL.h>
#include <SDL_image.h>
#include <time.h>
#include <string.h>
#include <stdint.h>

#include "main.h"
#include "graphics.h"
#include "memory.h"

COLOR fillColor = 0;
COLOR strokeColor = 0;
int strokeWidth = 0;

void lua_setup_draw() {
    lua_pushinteger(L, SCREEN_WIDTH);
    lua_setglobal(L, "SCREEN_WIDTH");
    lua_pushinteger(L, SCREEN_HEIGHT);
    lua_setglobal(L, "SCREEN_HEIGHT");
}

void blend(unsigned char result[4], const unsigned char fg[4], const unsigned char bg[4]) {
    float alpha_fg = fg[3] / 255.0f;
    float alpha_bg = bg[3] / 255.0f;
    float alpha_out = alpha_fg + alpha_bg * (1 - alpha_fg);

    if (alpha_out > 0) {
        result[0] = (unsigned char)((fg[0] * alpha_fg + bg[0] * alpha_bg * (1 - alpha_fg)) / alpha_out);
        result[1] = (unsigned char)((fg[1] * alpha_fg + bg[1] * alpha_bg * (1 - alpha_fg)) / alpha_out);
        result[2] = (unsigned char)((fg[2] * alpha_fg + bg[2] * alpha_bg * (1 - alpha_fg)) / alpha_out);
    } else {
        result[0] = result[1] = result[2] = 0;
    }
    result[3] = (unsigned char)(alpha_out * 255);
}

int millis() {
    return clock() / (CLOCKS_PER_SEC / 1000);
}

void draw_sprite_advanced(int sourceX, int sourceY, int sourceW, int sourceH, int targetX, int targetY, int targetW, int targetH, int angle, FLIP flip) {
    SDL_Rect source;
    source.x = sourceX;
    source.y = sourceY;
    source.w = sourceW;
    source.h = sourceH;

    SDL_Rect target;
    target.x = targetX;
    target.y = targetY;
    target.w = targetW;
    target.h = targetH;

    SDL_RenderCopyEx(renderer, spritesheet, &source, &target, angle, NULL, flip);
}

void draw_sprite(int sourceX, int sourceY, int sourceW, int sourceH, int targetX, int targetY, int targetW, int targetH) {
    // draw_sprite_advanced(sourceX, sourceY, sourceW, sourceH, targetX, targetY, targetW, targetH, 0, FLIP_NONE);

    for (int y = 0; y < sourceH; ++y) {
        for (int x = 0; x < sourceW; ++x) {

            if(x + sourceX < 0 || x + sourceX >= SCREEN_WIDTH || y + sourceY < 0 || y + sourceY >= SCREEN_HEIGHT) {
                continue;
            }

            if(x + targetX < 0 || x + targetX >= SCREEN_WIDTH || y + targetY < 0 || y + targetY >= SCREEN_HEIGHT) {
                continue;
            }

            // set rgb values, keep in mind possible stretching
            Uint8 r = memory[MEM_SPRITESHEET_START + ((sourceY + y) * SCREEN_WIDTH + sourceX + x) * 4];
            Uint8 g = memory[MEM_SPRITESHEET_START + ((sourceY + y) * SCREEN_WIDTH + sourceX + x) * 4 + 1];
            Uint8 b = memory[MEM_SPRITESHEET_START + ((sourceY + y) * SCREEN_WIDTH + sourceX + x) * 4 + 2];
            Uint8 a = memory[MEM_SPRITESHEET_START + ((sourceY + y) * SCREEN_WIDTH + sourceX + x) * 4 + 3];
    
            memory[MEM_DISPLAY_START + ((targetY + y) * SCREEN_WIDTH + targetX + x) * 4] = r;
            memory[MEM_DISPLAY_START + ((targetY + y) * SCREEN_WIDTH + targetX + x) * 4 + 1] = g;
            memory[MEM_DISPLAY_START + ((targetY + y) * SCREEN_WIDTH + targetX + x) * 4 + 2] = b;
            memory[MEM_DISPLAY_START + ((targetY + y) * SCREEN_WIDTH + targetX + x) * 4 + 3] = a;
        }
    }

}

void draw_rect(int x, int y, int w, int h) {

    SDL_Rect rect;

    if (strokeWidth > 0) {

        SDL_SetRenderDrawColor(renderer, strokeColor & 0xFF, (strokeColor >> 8) & 0xFF, (strokeColor >> 16) & 0xFF, (strokeColor >> 24) & 0xFF);

        // top
        rect.x = x;
        rect.y = y;
        rect.w = w;
        rect.h = strokeWidth;
        SDL_RenderFillRect(renderer, &rect);

        // bottom
        rect.y = y + h - strokeWidth;
        SDL_RenderFillRect(renderer, &rect);

        // left
        rect.x = x;
        rect.y = y + strokeWidth;
        rect.w = strokeWidth;
        rect.h = h - strokeWidth * 2;
        SDL_RenderFillRect(renderer, &rect);

        // right
        rect.x = x + w - strokeWidth;
        SDL_RenderFillRect(renderer, &rect);

        rect.x = x + strokeWidth;
        rect.y = y + strokeWidth;
        rect.w = w - strokeWidth * 2;
        rect.h = h - strokeWidth * 2;
    }

    SDL_SetRenderDrawColor(renderer, fillColor & 0xFF, (fillColor >> 8) & 0xFF, (fillColor >> 16) & 0xFF, (fillColor >> 24) & 0xFF);
    SDL_RenderFillRect(renderer, &rect);
}

void set_stroke(int width, int r, int g, int b, int a) {
    strokeWidth = width >= 0 ? width : 0;
    strokeColor = (r & 0xFF) | ((g & 0xFF) << 8) | ((b & 0xFF) << 16) | ((a & 0xFF) << 24);
}

void set_fill(int r, int g, int b, int a) {
    fillColor = (r & 0xFF) | ((g & 0xFF) << 8) | ((b & 0xFF) << 16) | ((a & 0xFF) << 24);
}

void draw_pixel(int x, int y) {
    uint32_t* pixel = &memory[MEM_DISPLAY_START + (x + (y * SCREEN_WIDTH)) * 4];
    uint32_t copy = *pixel;
    blend(pixel, &fillColor, &copy);
}