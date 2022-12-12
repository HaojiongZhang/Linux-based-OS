#ifndef TEXTMODE_H
#define TEXTMODE_H

#include "lib.h"

// enable the cursor 
extern void enable_cursor(uint8_t cursor_start, uint8_t cursor_end);
// disable the cursor
extern void disable_cursor();
// update the cursor position
extern void update_cursor(int x, int y);
// get the cursor_position
extern uint16_t get_cursor_position(void);

#endif /*TEXTMODE_H*/
