
#include <stdbool.h>

#include "main.h"
#include "input.h"

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

bool input_btn(BUTTON b) {
	const char* state = (const char*)SDL_GetKeyboardState(NULL);
	return state[(SDL_Scancode)b] == 1;
}