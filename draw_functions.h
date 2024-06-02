
#ifndef DRAW_FUNCTIONS_H
#define DRAW_FUNCTIONS_H

#include <SDL.h>

int millis();
void draw_sprite_advanced(int sourceX, int sourceY, int sourceW, int sourceH, int targetX, int targetY, int targetW, int targetH, int angle, SDL_RendererFlip flip);
void draw_sprite(int sourceX, int sourceY, int sourceW, int sourceH, int targetX, int targetY, int targetW, int targetH);

#endif