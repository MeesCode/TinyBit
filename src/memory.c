
#include <stdint.h>
#include <string.h>
#include "memory.h"
#include "main.h"


uint8_t memory[MEM_SIZE];

void memory_init() {
    memset(memory, 0, MEM_SIZE);
}

