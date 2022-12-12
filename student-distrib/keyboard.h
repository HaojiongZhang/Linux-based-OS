#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "lib.h"

extern int8_t ctrl_c;
// initialize irq port
extern void keyboard_init();

// handle keyboard interruption
extern void keyboard_interrupt();

// Checking if it's a upper character 
extern int32_t in_upper_alphabet(char ascii);

extern int32_t terminal_init();

extern int32_t terminal_open(const uint8_t *filename);

extern int32_t terminal_close(int32_t fd);

extern int32_t terminal_read(int32_t fd, void * buf, int32_t n_bytes);

extern int32_t terminal_write(int32_t fd, void * buf, int32_t n_bytes);

extern void terminal_switch(uint32_t terminal_num);

extern void init_terminal();
#endif /* KEYBOARD_H */
