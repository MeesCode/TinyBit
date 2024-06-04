#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

#define MEM_SIZE 0x80000 // 512KiB
#define MEM_HEADER_START 0x0000
#define MEM_HEADER_SIZE 0x0400
#define MEM_SPRITESHEET_START 0x0400
#define MEM_SPRITESHEET_SIZE 0x10000
#define MEM_DISPLAY_START 0x10400
#define MEM_DISPLAY_SIZE 0x10000
#define MEM_USER_START 0x20400

extern uint8_t memory[MEM_SIZE];

void memory_init();
void memory_setup_memory();
void mem_copy(int dst, int src, int size);

#endif