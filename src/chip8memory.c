#include "chip8memory.h"
#include <assert.h>

static void chip8_is_memory_in_bounds(int index)
{
    /* Check whether the index is withing the memory bounds. */
    assert(index >= 0 && index < CHIP8_MEMORY_SIZE);
}

void chip8_memory_set(struct chip8_memory *memory, int index, 
                      unsigned char val)
{
    chip8_is_memory_in_bounds(index);
    memory->memory[index] = val;
}

unsigned char chip8_memory_get(struct chip8_memory *memory, int index)
{
    chip8_is_memory_in_bounds(index);
    return memory->memory[index];
}

unsigned short chip8_memory_get_short(struct chip8_memory *memory, int index)
{
    unsigned char byte1 = chip8_memory_get(memory, index);
    unsigned char byte2 = chip8_memory_get(memory, index + 1);
    /* This will allow us to read two bytes from memory. The op size is 2 
    bytes.*/
    return(byte1 << 8 | byte2);
}
