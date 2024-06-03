
#include <SDL.h>
#include <SDL_image.h>
#include <time.h>

#include "main.h"

COLOR fillColor = 0;
COLOR strokeColor = 0;
int strokeWidth = 0;

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
    draw_sprite_advanced(sourceX, sourceY, sourceW, sourceH, targetX, targetY, targetW, targetH, 0, FLIP_NONE);
}

void draw_rect(int x, int y, int w, int h) {

    SDL_Rect rect;

    if (strokeWidth > 0) {

        // top
        rect.x = x;
        rect.y = y;
        rect.w = w;
        rect.h = strokeWidth;
        SDL_SetRenderDrawColor(renderer, strokeColor & 0xFF, (strokeColor >> 8) & 0xFF, (strokeColor >> 16) & 0xFF, (strokeColor >> 24) & 0xFF);
        SDL_RenderFillRect(renderer, &rect);

        // bottom
        rect.y = y + h - strokeWidth;
        SDL_SetRenderDrawColor(renderer, strokeColor & 0xFF, (strokeColor >> 8) & 0xFF, (strokeColor >> 16) & 0xFF, (strokeColor >> 24) & 0xFF);
        SDL_RenderFillRect(renderer, &rect);

        // left
        rect.x = x;
        rect.y = y + strokeWidth;
        rect.w = strokeWidth;
        rect.h = h - strokeWidth * 2;
        SDL_SetRenderDrawColor(renderer, strokeColor & 0xFF, (strokeColor >> 8) & 0xFF, (strokeColor >> 16) & 0xFF, (strokeColor >> 24) & 0xFF);
        SDL_RenderFillRect(renderer, &rect);

        // right
        rect.x = x + w - strokeWidth;
        SDL_SetRenderDrawColor(renderer, strokeColor & 0xFF, (strokeColor >> 8) & 0xFF, (strokeColor >> 16) & 0xFF, (strokeColor >> 24) & 0xFF);
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