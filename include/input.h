#ifndef input_H
#define input_H

#include <stdbool.h>

typedef enum {
	X = SDL_SCANCODE_X,
	Z = SDL_SCANCODE_Z,
	UP = SDL_SCANCODE_UP,
	DOWN = SDL_SCANCODE_DOWN,
	LEFT = SDL_SCANCODE_LEFT,
	RIGHT = SDL_SCANCODE_RIGHT,
	START = SDL_SCANCODE_ESCAPE
} BUTTON;

void lua_setup_input();

bool input_btn(BUTTON btn);

#endif
