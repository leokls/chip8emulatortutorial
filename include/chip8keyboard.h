#ifndef CHIP8KEYBOARD_H
#define CHIP8KEYBOARD_H

/*
The computers which originally used the Chip-8 Language had a 16-key 
hexadecimal keypad with the following layout:

1	2	3	C
4	5	6	D
7	8	9	E
A	0	B	F
*/

#include <stdbool.h>
#include "config.h"

struct chip8_keyboard
{
    _Bool keyboard[CHIP8_TOTAL_KEYS];
    const char *keyboard_map;
};

void chip8_keyboard_set_map(struct chip8_keyboard *keyboard, const char *map);
int chip8_keyboard_map(struct chip8_keyboard *keyboard, char key);
void chip8_keyboard_down(struct chip8_keyboard *keyboard, int key);
void chip8_keyboard_up(struct chip8_keyboard *keyboard, int key);
_Bool chip8_keyboard_is_down(struct chip8_keyboard *keyboard, int key);

#endif
