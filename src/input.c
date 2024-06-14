
#include <stdbool.h>

#include "main.h"
#include "input.h"

bool prev_button_state[7];

void lua_setup_input() {
	lua_pushinteger(L, X);
	lua_setglobal(L, "X");
	lua_pushinteger(L, Z);
	lua_setglobal(L, "Z");
	lua_pushinteger(L, UP);
	lua_setglobal(L, "UP");
	lua_pushinteger(L, DOWN);
	lua_setglobal(L, "DOWN");
	lua_pushinteger(L, LEFT);
	lua_setglobal(L, "LEFT");
	lua_pushinteger(L, RIGHT);
	lua_setglobal(L, "RIGHT");
	lua_pushinteger(L, START);
	lua_setglobal(L, "START");
}

BUTTON button_down(BUTTON b){

	const char* state = (const char*)SDL_GetKeyboardState(NULL);

	switch(b){
		case X: return state[(SDL_Scancode) SDL_SCANCODE_X] == 1;
		case Z: return state[(SDL_Scancode) SDL_SCANCODE_Z] == 1;
		case UP: return state[(SDL_Scancode) SDL_SCANCODE_UP] == 1;
		case DOWN: return state[(SDL_Scancode) SDL_SCANCODE_DOWN] == 1;
		case LEFT: return state[(SDL_Scancode) SDL_SCANCODE_LEFT] == 1;
		case RIGHT: return state[(SDL_Scancode) SDL_SCANCODE_RIGHT] == 1;
		case START: return state[(SDL_Scancode) SDL_SCANCODE_ESCAPE] == 1;	
	}

	return false;
}

void save_button_state(){
	for(int i = 0; i <= 7; i++){
		prev_button_state[i] = button_down((BUTTON)i);
	}
}

bool input_btn(BUTTON b) {
	return button_down(b);
}

bool input_btnp(BUTTON b) {
	return button_down(b) && !prev_button_state[(int)b];
}