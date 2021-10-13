#include "chip8.h"
#include <memory.h>
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include "SDL2/SDL.h"

const char chip8_default_character_set[] = 
{
    0xf0, 0x90, 0x90, 0x90, 0xf0,          /* "0" */
    0x20, 0x60, 0x20, 0x20, 0x70,          /* "1" */
    0xf0, 0x10, 0xf0, 0x80, 0xf0,          /* "2" */
    0xf0, 0x10, 0xf0, 0x10, 0xf0,          /* "3" */
    0x90, 0x90, 0xf0, 0x10, 0x10,          /* "4" */
    0xf0, 0x80, 0xf0, 0x10, 0xf0,          /* "5" */
    0xf0, 0x80, 0xf0, 0x90, 0xf0,          /* "6" */
    0xf0, 0x10, 0x20, 0x40, 0x40,          /* "7" */
    0xf0, 0x90, 0xf0, 0x90, 0xf0,          /* "8" */
    0xf0, 0x90, 0xf0, 0x10, 0xf0,          /* "9" */
    0xf0, 0x90, 0xf0, 0x90, 0x90,          /* "A" */
    0xe0, 0x90, 0xe0, 0x90, 0xe0,          /* "B" */
    0xf0, 0x80, 0x80, 0x80, 0xf0,          /* "C" */
    0xe0, 0x90, 0x90, 0x90, 0xe0,          /* "D" */
    0xf0, 0x80, 0xf0, 0x80, 0xf0,          /* "E" */
    0xf0, 0x80, 0xf0, 0x80, 0x80,          /* "F" */
};

void chip8_init(struct chip8 *chip8)
{
    memset(chip8, 0, sizeof(struct chip8));
    
    /* Copy the character set into the correct location in memory, as per:
    "Programs may also refer to a group of sprites representing the hexadecimal 
    digits 0 through F. These sprites are 5 bytes long, or 8x5 pixels. The data 
    should be stored in the interpreter area of Chip-8 memory 
    (0x000 to 0x1FF)." */
    memcpy(&chip8->memory.memory, chip8_default_character_set, 
                                  sizeof(chip8_default_character_set));
}

/*
Memory Map:
+---------------+= 0xFFF (4095) End of Chip-8 RAM
|               |
|               |
|               |
|               |
|               |
| 0x200 to 0xFFF|
|     Chip-8    |
| Program / Data|
|     Space     |
|               |
|               |
|               |
+- - - - - - - -+= 0x600 (1536) Start of ETI 660 Chip-8 programs
|               |
|               |
|               |
+---------------+= 0x200 (512) Start of most Chip-8 programs
| 0x000 to 0x1FF|
| Reserved for  |
|  interpreter  |
+---------------+= 0x000 (0) Start of Chip-8 RAM
*/

void chip8_load(struct chip8 *chip8, const char *buf, size_t size)
{
    /* 0x200 (512) Start of most Chip-8 programs. */
    assert(size + CHIP8_PROGRAM_LOAD_ADDRESS < CHIP8_MEMORY_SIZE);
    memcpy(&chip8->memory.memory[CHIP8_PROGRAM_LOAD_ADDRESS], buf, size);
    chip8->registers.PC = CHIP8_PROGRAM_LOAD_ADDRESS;
}

static void chip8_exec_extended_eight(struct chip8 *chip8, 
                                      unsigned short opcode)
{
    unsigned char x = (opcode >> 8) & 0x000f;
    unsigned char y = (opcode >> 4) & 0x000f;    
    unsigned char final_four_bits = opcode & 0x000f;  /* operation to run*/
    unsigned short tmp = 0;
    switch(final_four_bits)
    {

        case 0x00:  /* 8xy0 - LD Vx, Vy. Set Vx = Vy. */
            /* Stores the value of register Vy in register Vx. */
            chip8->registers.V[x] = chip8->registers.V[y];
        break;

        case 0x01:  /* 8xy1 - OR Vx, Vy. Set Vx = Vx OR Vy. */ 
            /* Performs a bitwise OR on the values of Vx and Vy, then stores 
            the result in Vx. A bitwise OR compares the corrseponding bits from 
            two values, and if either bit is 1, then the same bit in the result 
            is also 1. Otherwise, it is 0. */
            chip8->registers.V[x] = (chip8->registers.V[x] | 
                                     chip8->registers.V[y]);
        break;

        case 0x02:  /* 8xy2 - AND Vx, Vy. Set Vx = Vx AND Vy.*/
            /* Performs a bitwise AND on the values of Vx and Vy, then stores 
            the result in Vx. A bitwise AND compares the corrseponding bits 
            from two values, and if both bits are 1, then the same bit in the 
            result is also 1. Otherwise, it is 0. */
            chip8->registers.V[x] = (chip8->registers.V[x] & 
                                     chip8->registers.V[y]);    
        break;

        case 0x03:  /* 8xy3 - XOR Vx, Vy. Set Vx = Vx XOR Vy. */
            /* Performs a bitwise exclusive OR on the values of Vx and Vy, then 
            stores the result in Vx. An exclusive OR compares the corrseponding 
            bits from two values, and if the bits are not both the same, then 
            the corresponding bit in the result is set to 1. Otherwise, it is 
            0. */
            chip8->registers.V[x] = (chip8->registers.V[x] ^ 
                                     chip8->registers.V[y]); 
        break;

        case 0x04:  /* 8xy4 - ADD Vx, Vy. Set Vx = Vx + Vy, set VF = carry. */
        {    
            /* The values of Vx and Vy are added together. If the result is 
            greater than 8 bits (i.e., > 255,) VF is set to 1, otherwise 0. 
            Only the lowest 8 bits of the result are kept, and stored in Vx. */
            tmp = chip8->registers.V[x] + chip8->registers.V[y];
            chip8->registers.V[0x0f] = false;
            if (tmp > 0xff)
            {
                chip8->registers.V[0x0f] = true;
            }
            chip8->registers.V[x] = tmp;
        }
        break;

        case 0x05:  /* 8xy5 - SUB Vx, Vy. Set Vx = Vx-Vy, set VF =NOT borrow.*/
        {    
            /* If Vx > Vy, then VF is set to 1, otherwise 0. Then Vy is 
            subtracted from Vx, and the results stored in Vx. */
            chip8->registers.V[0x0f] = false;
            if (chip8->registers.V[x] > chip8->registers.V[y])
                chip8->registers.V[0x0f] = true;
            chip8->registers.V[x] -= chip8->registers.V[y];
        }
        break;

        case 0x06:  /* 8xy6 - SHR Vx {, Vy}. Set Vx = Vx SHR 1. */
            
            /* If the least-significant bit of Vx is 1, then VF is set to 1, 
            otherwise 0. Then Vx is divided by 2. */
        {    
            chip8->registers.V[0x0f] = chip8->registers.V[x] & 0b00000001;
            chip8->registers.V[x] /= 2;
        }
        break;

        case 0x07:  /* 8xy7 - SUBN Vx, Vy. Set Vx=Vy - Vx, set VF=NOT borrow.*/
        {    
            /* If Vy > Vx, then VF is set to 1, otherwise 0. 
            Then Vx is subtracted from Vy, and the results stored in Vx.*/
            chip8->registers.V[x] = (chip8->registers.V[y] > 
                                     chip8->registers.V[x]);
            chip8->registers.V[x] = (chip8->registers.V[y] - 
                                     chip8->registers.V[x]);
        }
        break; 

        case 0x0E:  /* 8xyE - SHL Vx {, Vy}. Set Vx = Vx SHL 1. */
        {    
            /* If the most-significant bit of Vx is 1, then VF is set to 1, 
            otherwise to 0. Then Vx is multiplied by 2. */
            chip8->registers.V[0x0f] = chip8->registers.V[x] & 0b10000000;
            chip8->registers.V[x] *= 2;
        }
        break;
    }
}

static char chip8_wait_for_key_press(struct chip8 *chip8)
{
    SDL_Event event;
    while(SDL_WaitEvent(&event))
    {
        if (event.type != SDL_KEYDOWN)
            continue;

        char c = event.key.keysym.sym;
        char chip8_key = chip8_keyboard_map(&chip8->keyboard, c);
        if (chip8_key != -1)
        {
            return chip8_key;
        }
    }    
    return(-1);
}

static void chip8_exec_extended_F(struct chip8 *chip8, unsigned short opcode)
{
    unsigned char x = (opcode >> 8) & 0x000f;
    switch (opcode & 0x00ff)
    {
        case 0x07:  /* Fx07 - LD Vx, DT. Set Vx = delay timer value. */
        {
            /* The value of DT is placed into Vx. */
            chip8->registers.V[x] = chip8->registers.delay_timer;
        }
        break;

        case 0x0A:  /* Fx0A - LD Vx, K. Wait for a key press, store the value 
            of the key in Vx.*/
        {
            /* All execution stops until a key is pressed, then the value of 
            that key is stored in Vx.*/
            char pressed_key = chip8_wait_for_key_press(chip8);
            chip8->registers.V[x] = pressed_key;
        }
        break;

        case 0x15:  /* Fx15 - LD DT, Vx. Set delay timer = Vx.*/ 
        {
            /* DT is set equal to the value of Vx. */
            chip8->registers.delay_timer = chip8->registers.V[x];            
        }
        break;

        case 0x18:  /* Fx18 - LD ST, Vx. Set sound timer = Vx. */

            /* ST is set equal to the value of Vx. */
            chip8->registers.sound_timer = chip8->registers.V[x];
        break;

        case 0x1E:  /* Fx1E - ADD I, Vx. Set I = I + Vx. */ 
        {
            /* The values of I and Vx are added, and the results are stored 
            in I.*/
            chip8->registers.I += chip8->registers.V[x];
        }
        break;

        case 0x29:  /* Fx29 - LD F,Vx. Set I=location of sprite for digit Vx.*/

            /* The value of I is set to the location for the hexadecimal sprite 
            corresponding to the value of Vx. See section 2.4, Display, for 
            more information on the Chip-8 hexadecimal font. */
            chip8->registers.I = chip8->registers.V[x] * 
                                 CHIP8_DEFAULT_SPRITE_HEIGHT;
        break;

        case 0x33:  /* Fx33 - LD B, Vx. Store BCD representation of Vx in 
            memory locations I, I+1, and I+2. */
        {
            /* The interpreter takes the decimal value of Vx, and places the 
            hundreds digit in memory at location in I, the tens digit at 
            location I+1, and the ones digit at location I+2. */
            unsigned char hundreds = chip8->registers.V[x] / 100;
            unsigned char tens = chip8->registers.V[x] / 10 % 10;    
            unsigned char units = chip8->registers.V[x] % 10;     
            chip8_memory_set(&chip8->memory, chip8->registers.I, hundreds);
            chip8_memory_set(&chip8->memory, chip8->registers.I + 1, tens);
            chip8_memory_set(&chip8->memory, chip8->registers.I + 2, units);
        }
    
        case 0x55:  /* Fx55 - LD [I],Vx. Store registers V0 through Vx in 
            memory starting at location I.  */
        {
            /* The interpreter copies the values of registers V0 through Vx 
            into memory, starting at the address in I.*/
            for (int i = 0; i <= x; i++)
            {
                chip8_memory_set(&chip8->memory, chip8->registers.I + i, 
                                                        chip8->registers.V[i]);
            }
        }
        break;

        case 0x65:  /* Fx65-LD Vx,[I]. Read registers V0 through Vx from memory 
            starting at location I.  */
        {
            /* The interpreter reads values from memory starting at location I 
            into registers V0 through Vx. */
            for (int i = 0; i <= x; i++)
            {
                chip8->registers.V[i] = chip8_memory_get(&chip8->memory, 
                                                       chip8->registers.I + i);
            }
        }
        break;
    }
}

static void chip8_exec_extended(struct chip8 *chip8, unsigned short opcode)
{
    /* 
    nnn or addr - A 12-bit value, the lowest 12 bits of the instruction
    n or nibble - A 4-bit value, the lowest 4 bits of the instruction
    x - A 4-bit value, the lower 4 bits of the high byte of the instruction
    y - A 4-bit value, the upper 4 bits of the low byte of the instruction
    kk or byte - An 8-bit value, the lowest 8 bits of the instruction
    */
    unsigned short nnn = opcode & 0x0fff; 
    unsigned char x = (opcode >> 8) & 0x000f;
    unsigned char y = (opcode >> 4) & 0x000f;
    unsigned char kk = opcode & 0x00ff;
    unsigned char n = opcode & 0x000f;
    switch(opcode & 0xf000)  /* We see only the first four bits. */
    {
        
        case 0x1000:  /* 1nnn - JP addr. Jump to location nnn */  

            /* The interpreter sets the PC to nnn.*/  
            chip8->registers.PC = nnn;            
        break;
        
        case 0x2000:  /* 2 nnn - CALL addr. Call subroutine at location nnn. */ 
        {   
            /* The interpreter increments the stack pointer, then puts the 
            current PC on the top of the stack. The PC is then set to nnn.*/
            chip8_stack_push(chip8, chip8->registers.PC);
            chip8->registers.PC = nnn;
        }
        break;

        case 0x3000:  /*3xkk - SE Vx, byte. Skip next instruction if Vx = kk.*/
        {
            /*  The interpreter compares register Vx to kkk, and if they are 
            equal, increments the program counter by 2.*/
            if (chip8->registers.V[x] == kk)
            {
                chip8->registers.PC += 2;  /* Each instruction is 2 bytes. */
            }
        }
        break;

        case 0x4000:  /*4xkk -SNE Vx,byte. Skip next instruction if Vx != kk.*/
        {
            /* The interpreter compares register Vx to kk, and if they are not 
            equal, increments the program counter by 2.*/
            if (chip8->registers.V[x] != kk)
            {
                chip8->registers.PC += 2;
            }
        }
        break;

        case 0x5000:  /* 5xy0-SE Vx,Vy. Skip next instruction if Vx = Vy. */
        {
            /* The interpreter compares register Vx to register Vy, and if they 
            are equal, increments the program counter by 2. */
            if (chip8->registers.V[x] == chip8->registers.V[y])
            {
                chip8->registers.PC += 2;
            }
        }
        break;

        case 0x6000: /* 6xkk - LD Vx, byte. Set Vx = kk.*/ 

            /* The interpreter puts the value kk into register Vx. */
            chip8->registers.V[x] = kk;
        break;

        case 0x7000: /* 7xkk - ADD Vx, byte. Set Vx = Vx + kk. */

            /* Adds the value kk to the value of register Vx, then stores the 
            result in Vx. */
            chip8->registers.V[x] += kk;
        break;

        case 0x8000:
            chip8_exec_extended_eight(chip8, opcode);            
        break;

        case 0x9000:  /* 9xy0 - SNE Vx,Vy. Skip next instruction if Vx != Vy.*/
        {
            /* The values of Vx and Vy are compared, and if they are not equal, 
            the program counter is increased by 2 */
            if (chip8->registers.V[x] != chip8->registers.V[y])
            {
                chip8->registers.PC += 2;
            }
        }            
        break;  

        case 0xA000:  /* Annn - LD I, addr. Set I = nnn. */

            /* The value of register I is set to nnn. */   
            chip8->registers.I = nnn; 
        break;

        case 0xB000:  /* Bnnn - JP V0, addr. Jump to location nnn + V0. */

            /* The program counter is set to nnn plus the value of V0. */
            chip8->registers.PC = nnn + chip8->registers.V[0x00];        
        break;

        case 0xC000:  /* Cxkk - RND Vx, byte. Set Vx = random byte AND kk. */
        {
            /* The interpreter generates a random number from 0 to 255, which 
            is then ANDed with the value kk. The results are stored in Vx. 
            See instruction 8xy2 for more information on AND. */

            /* seed for a random number generator*/ 
            srand(clock());
            chip8->registers.V[x] = (rand() % 255) & kk;         
        }
        break;

        case 0xD000:  /* Dxyn - DRW Vx, Vy, nibble.  */
            /* Display n-byte sprite starting at memory location I at (Vx, Vy), 
            set VF = collision. */

            /* The interpreter reads n bytes from memory, starting at the 
            address stored in I. These bytes are then displayed as sprites on 
            screen at coordinates (Vx, Vy). Sprites are XORed onto the existing 
            screen. If this causes any pixels to be erased, VF is set to 1, 
            otherwise it is set to 0. If the sprite is positioned so part of it 
            is outside the coordinates of the display, it wraps around to the 
            opposite side of the screen. See instruction 8xy3 for more 
            information on XOR, and section 2.4, Display, for more information 
            on the Chip-8 screen and sprites. */
            {
                const char *sprite = (const char*) 
                                     &chip8->memory.memory[chip8->registers.I];
                chip8->registers.V[0x0f] = chip8_screen_draw_sprite(
                    &chip8->screen, chip8->registers.V[x], 
                    chip8->registers.V[y], sprite, n);
            }
        break;

        case 0xE000:  /* Keyboard operations. */
            {   
                switch(opcode & 0x00ff)
                {
                    /* Ex9E - SKP Vx. Skip next instruction if key with the 
                    value of Vx is pressed.  */
                    case 0x9e:
                    {
                        /* Checks the keyboard, and if the key corresponding to 
                        the value of Vx is currently in the down position, PC 
                        is increased by 2. */
                        if (chip8_keyboard_is_down(&chip8->keyboard, 
                                                        chip8->registers.V[x]))
                            chip8->registers.PC += 2;
                    }
                    break;

                    /* ExA1 - SKNP Vx. Skip next instruction if key with the 
                    value of Vx is not pressed. */
                    case 0xa1:
                    {
                        /* Checks the keyboard, and if the key corresponding to 
                        the value of Vx is currently in the up position, PC is 
                        increased by 2. */
                        if (!chip8_keyboard_is_down(&chip8->keyboard, 
                                                        chip8->registers.V[x]))
                        {
                            chip8->registers.PC += 2;            
                        }
                    }
                    break;
                }
            }            
        break;

        case 0xF000:  
            chip8_exec_extended_F(chip8, opcode);
        break;
    }
}

void chip8_exec(struct chip8 *chip8, unsigned short opcode)
{
    /* Standard Chip-8 Instructions. */
    switch(opcode)
    {
        case 0x00E0:  /* CLS. Clear the display. */
            chip8_screen_clear(&chip8->screen);
        break;
     
        case 0x00EE:  /* RET. Return from the subroutine. */

            /* The interpreter sets the program counter to the address at the 
            top of the stack, then subtracts 1 from the stack pointer.*/
            chip8->registers.PC = chip8_stack_pop(chip8); 
        break;

        default:
            chip8_exec_extended(chip8, opcode); 
    }
}
