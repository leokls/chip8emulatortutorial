#ifndef CONFIG_H
#define CONFIG_H

#define EMULATOR_WINDOW_TITLE "Chip8 Emulator"

/* The Chip-8 language is capable of accessing up to 4KB (4,096 bytes) of RAM, 
from location 0x000 (0) to 0xFFF (4095).*/
#define CHIP8_MEMORY_SIZE 4096

/* The original implementation of the Chip-8 language used a 64x32-pixel 
monochrome display. */
#define CHIP8_WIDTH 64
#define CHIP8_HEIGHT 32
#define CHIP8_WINDOW_MULTIPLIER 10
#define CHIP8_TOTAL_DATA_REGISTERS 16
#define CHIP8_TOTAL_STACK_DEPTH 16
#define CHIP8_PROGRAM_LOAD_ADDRESS 0x200
#define CHIP8_CHARACTER_SET_LOAD_ADDRESS 0x00
#define CHIP8_TOTAL_KEYS 16
#define CHIP8_DEFAULT_SPRITE_HEIGHT 5

#endif
