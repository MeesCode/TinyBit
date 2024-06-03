#ifndef MAIN_H
#define MAIN_H

#include <SDL.h>
#include "lua.h"

#define FLIP_NONE SDL_FLIP_NONE
#define FLIP_HORIZONTAL SDL_FLIP_HORIZONTAL
#define FLIP_VERTICAL SDL_FLIP_VERTICAL

typedef SDL_RendererFlip FLIP;
typedef uint32_t COLOR;

extern SDL_Renderer* renderer;
extern SDL_Texture* background;
extern SDL_Texture* spritesheet;
extern SDL_Window* window;
extern lua_State* L;

void destroyApplication();

#endif