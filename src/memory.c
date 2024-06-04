
#include <stdint.h>
#include "memory.h"
#include "main.h"

#define MEMORY_SIZE 0x10000 // 64KB

uint8_t memory[MEMORY_SIZE];

void memory_init() {
    memset(memory, 0, MEMORY_SIZE);
}

