#ifndef MAIN_H
#define MAIN_H

#include <SDL.h>
#include "lua/lua.h"

extern SDL_Renderer* renderer;
extern SDL_Texture* background;
extern SDL_Texture* spritesheet;
extern SDL_Window* window;
extern lua_State* L;

void destroyApplication();

#endif