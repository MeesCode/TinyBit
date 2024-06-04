
#include <stdint.h>
#include <string.h>
#include "memory.h"
#include "main.h"


uint8_t memory[MEM_SIZE];

void memory_init() {
    memset(memory, 0, MEM_SIZE);

    //lua_pushinteger(L, MEM_SIZE);
    //lua_setglobal(L, "MEM_SIZE");
    //lua_pushinteger(L, MEM_HEADER_START);
    //lua_setglobal(L, "MEM_HEADER_START");
    //lua_pushinteger(L, MEM_HEADER_SIZE);
    //lua_setglobal(L, "MEM_HEADER_SIZE");
    //lua_pushinteger(L, MEM_SPRITESHEET_START);
    //lua_setglobal(L, "MEM_SPRITESHEET_START");
    //lua_pushinteger(L, MEM_SPRITESHEET_SIZE);
    //lua_setglobal(L, "MEM_SPRITESHEET_SIZE");
    //lua_pushinteger(L, MEM_DISPLAY_START);
    //lua_setglobal(L, "MEM_DISPLAY_START");
    //lua_pushinteger(L, MEM_DISPLAY_SIZE);
    //lua_setglobal(L, "MEM_DISPLAY_SIZE");
    //lua_pushinteger(L, MEM_USER_START);
    //lua_setglobal(L, "MEM_USER_START");

}

void mem_copy(int dst, int src, int size) {
    if (dst + size > MEM_SIZE || src + size > MEM_SIZE) {
        return;
    }
    memcpy(&memory[dst], &memory[src], size);
}