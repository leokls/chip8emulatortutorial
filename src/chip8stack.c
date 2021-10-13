#include "chip8stack.h"
#include "chip8.h"
#include <assert.h>

static void chip8_stack_in_bounds(struct chip8 *chip8)
{
    /* Ensure that the functions do not overflow the stack array. */
    assert(chip8->registers.SP < sizeof(chip8->stack.stack));        
}

void chip8_stack_push(struct chip8 *chip8, unsigned short val)
{
    /* Increment the stack pointer. */  
    chip8->registers.SP += 1; 

    chip8_stack_in_bounds(chip8);

    /* The stack pointer (SP) can be 8-bit, it is used to point to the topmost 
    level of the stack.*/
    chip8->stack.stack[chip8->registers.SP] = val;               
}

unsigned short chip8_stack_pop(struct chip8 *chip8)
{
    chip8_stack_in_bounds(chip8);
    
    unsigned short res = chip8->stack.stack[chip8->registers.SP];

    /* Decrement the stack pointer. */
    chip8->registers.SP -= 1;

    return res;
}
