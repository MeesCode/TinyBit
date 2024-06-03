
#include <SDL.h>
#include <SDL_image.h>
#include <time.h>

#include "main.h"

int millis() {
    return clock() / (CLOCKS_PER_SEC / 1000);
}

void draw_sprite_advanced(int sourceX, int sourceY, int sourceW, int sourceH, int targetX, int targetY, int targetW, int targetH, int angle, SDL_RendererFlip flip) {
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
    draw_sprite_advanced(sourceX, sourceY, sourceW, sourceH, targetX, targetY, targetW, targetH, 0, SDL_FLIP_NONE);
}
