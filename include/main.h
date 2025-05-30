#ifndef MAIN_H
#define MAIN_H

#ifdef _WIN32
    #include <SDL.h>
#else
    #include <SDL2/SDL.h>
#endif

#include "lua.h"

#define FLIP_NONE SDL_FLIP_NONE
#define FLIP_HORIZONTAL SDL_FLIP_HORIZONTAL
#define FLIP_VERTICAL SDL_FLIP_VERTICAL

// Screen dimensions
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 128

// cartridge dimensions
#define CARTRIDGE_WIDTH 256
#define CARTRIDGE_HEIGHT 300

typedef SDL_RendererFlip FLIP;

extern SDL_Renderer* renderer;
extern SDL_Texture* render_target;
extern lua_State* L;

#endif