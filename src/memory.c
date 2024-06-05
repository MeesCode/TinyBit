
#include <stdint.h>
#include <string.h>
#include "memory.h"
#include "main.h"


uint8_t memory[MEM_SIZE];

void memory_init() {
    memset(memory, 0, MEM_SIZE);
}

void lua_setup_memory() {
    lua_pushinteger(L, MEM_SIZE);
    lua_setglobal(L, "MEM_SIZE");
    lua_pushinteger(L, MEM_HEADER_START);
    lua_setglobal(L, "MEM_HEADER_START");
    lua_pushinteger(L, MEM_HEADER_SIZE);
    lua_setglobal(L, "MEM_HEADER_SIZE");
    lua_pushinteger(L, MEM_SPRITESHEET_START);
    lua_setglobal(L, "MEM_SPRITESHEET_START");
    lua_pushinteger(L, MEM_SPRITESHEET_SIZE);
    lua_setglobal(L, "MEM_SPRITESHEET_SIZE");
    lua_pushinteger(L, MEM_DISPLAY_START);
    lua_setglobal(L, "MEM_DISPLAY_START");
    lua_pushinteger(L, MEM_DISPLAY_SIZE);
    lua_setglobal(L, "MEM_DISPLAY_SIZE");
    lua_pushinteger(L, MEM_USER_START);
    lua_setglobal(L, "MEM_USER_START");
}

void mem_copy(int dst, int src, int size) {
    if (dst + size > MEM_SIZE || src + size > MEM_SIZE) {
        return;
    }
    memcpy(&memory[dst], &memory[src], size);
}

uint8_t mem_peek(int dst) {
    if (dst < 0 || dst > MEM_SIZE) {
        return 0;
    }
    uint8_t* val = &memory[dst];
    return val;
}

void mem_poke(int dst, int val){
    if (dst < 0 || dst > MEM_SIZE) {
        return;
    }
    memory[dst] = val & 0xff;
}