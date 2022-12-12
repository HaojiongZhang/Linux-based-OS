#include "textmode.h"
#include "lib.h"
#define VGA_WIDTH 80
#define CURSOR_PORT 0x3D4
#define CURSOR_DATA 0x3D5

/*mainly reference from osdev textmode*/

/* 
 * void enable_cursor(cursor_start, cursor_end)
 *     DESCRIPTOR: enable the cursor 
 *                 set the cursor scanline start from cursor_start 
 *                 to cursor_end
 *     INPUT: start, end scanline
 *     OUTPUT: none
 *     
 */
void enable_cursor(uint8_t cursor_start, uint8_t cursor_end)
{
    // output 10 get the cursor data start
    outb(0x0A, CURSOR_PORT);
    // mask 3 4 bit
	outb((inb(CURSOR_DATA) & 0xC0) | cursor_start, CURSOR_DATA);
    // output 11 get the cursor data start
	outb(0x0B, CURSOR_PORT);
    // mask 4 3 2 bit
	outb((inb(CURSOR_DATA) & 0xE0) | cursor_end, CURSOR_DATA);
}

/*
 * void disable_cursor()
 *      DES: disable the cursor
 *      INPUT: none
 *      OUTPUT: none
 */
void disable_cursor()
{
    // output 10 get the cursor data start
	outb(0x0A, CURSOR_PORT);
    // output 10 get the cursor data end > start to disable cursor
	outb(0x20, CURSOR_DATA);
}

/*
 * void update_cursor()
 *      DES: update the cursor to (x,y)
 *      INPUT: x, y
 *      OUTPUT: none
 */
void update_cursor(int x, int y)
{
    // check if out of range
	if (x >= 80 || x < 0 || y >= 25 || y < 0){
        return;
    }
    uint16_t pos = y * VGA_WIDTH + x;
    // mask 4 bits
	outb(0x0F, CURSOR_PORT);
    // mask 8 bits
	outb((uint8_t) (pos & 0xFF), CURSOR_DATA);
    // mask 4 3 2 bit
	outb(0x0E, CURSOR_PORT);
    // shift to lower 8 bits and mask 8 bits
	outb((uint8_t) ((pos >> 8) & 0xFF), CURSOR_DATA);
}

/*
 *  uint16_t get_cursor_position(void)
 *      DES: get the cursor position
 *      INPUT: none
 *      OUTPUT: memory offset of cursor position
 */
uint16_t get_cursor_position(void)
{
    uint16_t pos = 0;
    // output 15 to get every bit data
    outb(0x0F, CURSOR_PORT);
    pos |= inb(CURSOR_DATA);
    // output 15 to get 2 3 4 bit data
    outb(0x0E, CURSOR_PORT);
    // get shift to the lower 8 bit
    pos |= ((uint16_t)inb(CURSOR_DATA)) << 8;
    return pos;
}



