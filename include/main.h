#ifndef MAIN_H
#define MAIN_H

#include <SDL.h>
#include "lua.h"

#define FLIP_NONE SDL_FLIP_NONE
#define FLIP_HORIZONTAL SDL_FLIP_HORIZONTAL
#define FLIP_VERTICAL SDL_FLIP_VERTICAL

// Screen dimensions
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 128

typedef SDL_RendererFlip FLIP;
typedef uint32_t COLOR;

extern SDL_Renderer* renderer;
extern SDL_Texture* spritesheet;
extern lua_State* L;

void destroyApplication();

#endif