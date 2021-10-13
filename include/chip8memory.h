#ifndef CHIP8MEMORY_H
#define CHIP8MEMORY_H

#include "config.h"

struct chip8_memory
{
    unsigned char memory[CHIP8_MEMORY_SIZE];
};

/* Implement function that talk to memory. */

/* Set the index in the "memory" array. */
void chip8_memory_set(struct chip8_memory *memory, int index, unsigned char val);

/* Get the value at the index in the "memory" array. */
unsigned char chip8_memory_get(struct chip8_memory *memory, int index);

unsigned short chip8_memory_get_short(struct chip8_memory *memory, int index);

#endif
