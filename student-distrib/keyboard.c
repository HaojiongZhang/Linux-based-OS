#include "keyboard.h"
#include "scancode_map.h"
#include "lib.h"
#include "i8259.h"
#include "textmode.h"
#include "cp3_syscall.h"



int8_t capLock;
int8_t left_shift;
int8_t right_shift;
int8_t ctrl;
int8_t alt;
int8_t ctrl_c;
/*keyboard buffer*/
// uint8_t terminals[dis_terminal].buffer[128];
// int terminals[dis_terminal].buffer_num;
int8_t terminal_read_status;

// special functional key scancode: 
#define left_shift_pressed      0x2A
#define left_shift_released     0xAA
#define right_shift_pressed     0x36
#define right_shift_released    0xB6

#define left_ctr_pressed        0x1D    
#define left_ctr_released       0x9D        

#define left_alt_pressed        0x38
#define left_alt_released       0xB8

#define cap_pressed             0x3A  
#define cap_released            0xBA

// special ascii keycode
#define backspace_pressed       0x0E
#define backspace_released      0x8E

#define tab_pressed             0x0F
#define tab_released            0x8F

#define enter_pressed           0x1C
#define enter_released          0x9C

#define L_pressed               0x26
#define C_pressed               0x2E

#define KEYBOARD_PORT           0x60
#define KEYBOARD_DATA           0x64
#define F1_pressed              0x3B
#define F2_pressed              0x3C
#define F3_pressed              0x3D

/*
Descriptor: this function is used to initialize keybord
all function keys assume unpressed at initialization.
input: none
return: none*/
void keyboard_init(uint32_t terminal){
    enable_irq(1);
    capLock = -1;
    left_shift = -1;
    right_shift = -1;
    ctrl = -1;  
    alt = -1;
    ctrl_c = -1;
    terminals[terminal].kb_num = 0;
    terminal_read_status = 0;
    clear();
}


/*
Descriptor: this function is used to handle keyboard interrupts
putc2 to screen depending on the read scancode
input: none
return: none*/
void keyboard_interrupt(){
    uint16_t scancode;
    char cur_ascii;
    cli();
    scancode = inb(KEYBOARD_PORT);  //read current scan code

    // check special function key press, only caps work for now.
    switch (scancode)
    {
    case cap_pressed:
        capLock *= -1;
        break;
    case left_shift_pressed:
        left_shift *= -1;
        break;
    case left_shift_released:
        left_shift *= -1;
        break;
    case right_shift_pressed:
        right_shift *= -1;
        break; 
    case right_shift_released:
        right_shift *= -1;
        break;
    case left_ctr_pressed:
        ctrl *= -1;
        break;
    case left_ctr_released:
        ctrl *= -1;
        break;
    case left_alt_pressed:
        alt *= -1;
        
        break;
    case left_alt_released:
        alt *= -1;
        break;
    // all special actions below will check if buffer full (>=128)
    case tab_pressed:
        if (terminals[dis_terminal - 1].kb_num >= 127){
            break;
        }
        terminals[dis_terminal - 1].buffer[terminals[dis_terminal - 1].kb_num] = '\t';
        terminals[dis_terminal - 1].kb_num++;
        putc2('\t');
        break;
    case backspace_pressed:
        if (terminals[dis_terminal - 1].kb_num >= 128){
            break;
        }
        if (terminals[dis_terminal - 1].kb_num == 0){
            break;
        }
        if(terminals[dis_terminal - 1].buffer[terminals[dis_terminal - 1].kb_num-1] == '\t'){
            putc2('\b');putc2('\b');putc2('\b');putc2('\b');
        }else{
            putc2('\b');
        }
        terminals[dis_terminal - 1].kb_num--;

        break;
    case enter_pressed:
        if (terminals[dis_terminal - 1].kb_num >= 128){
                break;
        }
        terminals[dis_terminal - 1].buffer[terminals[dis_terminal - 1].kb_num] = '\n';
        terminals[dis_terminal - 1].kb_num++;
        putc2('\n');
        // reading right now
        if (terminals[dis_terminal - 1].read_status == 1){
            terminals[dis_terminal - 1].read_status = 0;
        }
        else{
            terminals[dis_terminal - 1].kb_num = 0;
        }
        
        break;
    default:
        if (scancode >= 0x3E){
            break;//ignore scancodes above 3B(arrows, fn...)
        }
        if(alt == 1){
            if(scancode == F1_pressed) terminal_switch(1);
            else if(scancode == F2_pressed) terminal_switch(2);
            else if(scancode == F3_pressed) terminal_switch(3);
            break;
        }
        //enable ctrl+l/L clear screen
        if (ctrl == 1){
            if (scancode == L_pressed){
                if (terminals[dis_terminal - 1].read_status == 1){
                    terminals[dis_terminal - 1].kb_num = 0;
                }
                clear();
                break;
            }
            if (scancode == C_pressed){
                ctrl_c *= -1;
                putc2((uint8_t)'\n');
                break;
            }

        }
        if (terminals[dis_terminal - 1].kb_num >= 127){
            break;
        }
        if (left_shift == 1 || right_shift == 1){
            cur_ascii = scancode_map[scancode][1];
        }
        else{ 
            if (capLock == 1){
                cur_ascii = scancode_map[scancode][1];
                // checking whether the ascii is an upper alpha character
                if (! in_upper_alphabet (cur_ascii)){
                    cur_ascii = scancode_map[scancode][0];
                }
            }
            else{
                cur_ascii = scancode_map[scancode][0];
            }
        }
        terminals[dis_terminal - 1].buffer[terminals[dis_terminal - 1].kb_num] = cur_ascii;
        terminals[dis_terminal - 1].kb_num++;    
        putc2(cur_ascii);
        //put the current ascii to screen
        break;
    }
    // end of interrupt, notify via pic
    send_eoi(1); 
    sti();
}


/*
Descriptor: this function checks whether the 
scancode read is a upper case letter. return 1
if its upper, else return 0.
input: ascii to be examined
return: none*/
int32_t in_upper_alphabet(char ascii)
{
    int ascii_num;
    ascii_num = (int) ascii;
    if (ascii_num >= 65 && ascii_num <= 90){
        return 1;
    }
    else{
        return 0;
    }
}

/*
 * int terminal_init()
 *      DESCRIPTOR: terminal_initialization
 *      INTPUT: none
 *      OUTPUT: return 0
 *      EFFECT: make the read_Status into false
 * 
 * 
 */
int32_t terminal_init(){
    terminal_read_status = 0;
    return 0;
}

/*
 * int terminal_open()
 *      DESCRIPTOR: terminal_open
 *      INTPUT: none
 *      OUTPUT: return 0
 *      EFFECT: temporary none
 * 
 * 
 */
int32_t terminal_open(const uint8_t *filename){
    return 0;
}

/*
 * int terminal_close()
 *      DESCRIPTOR: terminal_close
 *      INTPUT: none
 *      OUTPUT: return 0
 *      EFFECT: temporary none
 * 
 * 
 */
int32_t terminal_close(int32_t fd){
    return 0;
}

/*
 * int terminal_read()
 *      DESCRIPTOR: terminal_read
 *      INTPUT: int32_t fd, char * buf, int32_t n_bytes
 *      OUTPUT: return number of bytes read from buffer
 *      EFFECT: read the terminals[dis_terminal].buffer into the terminal buffer
 * 
 * 
 */
int32_t terminal_read(int32_t fd, void * buf, int32_t n_bytes){
    int i;
    int min_bytes;
    if (fd != 0){
        return -1;
    }
    if (buf == NULL){
        return -1;
    }
    // out of the maximum range
    if (n_bytes < 0){
        return -1;
    }
    if (n_bytes == 0){
        return 0;
    }
    terminals[sche_terminal].kb_num = 0;
    // terminal read starts
    terminals[sche_terminal].read_status = 1;
    while(terminals[sche_terminal].read_status == 1){}

    // critical section copy
    cli();
    for (i = 0; i < n_bytes; i++){
        if (i < terminals[sche_terminal].kb_num){
            *(uint8_t *)(buf + i) = terminals[sche_terminal].buffer[i];
        }
        else{
            *(uint8_t *)(buf + i) = 0;
        }
    }
    // determine the lower output
    if (n_bytes > terminals[sche_terminal].kb_num){
        min_bytes = terminals[sche_terminal].kb_num;
    }
    else{
        min_bytes = n_bytes;
    }
    
    terminals[sche_terminal].kb_num = 0;
    //sti();
    return min_bytes;
}



/*
 * int terminal_write()
 *      DESCRIPTOR: terminal_write
 *      INTPUT: int32_t fd, const char * buf, int32_t n_bytes
 *      OUTPUT: return n_bytes write
 *      EFFECT: output the terminal buffer to screen
 * 
 * 
 */
int32_t terminal_write(int32_t fd, void * buf, int32_t n_bytes){
    cli();
    int i;
    if (fd != 1){
        return -1;
    }
    if (n_bytes < 0){
        return -1;
    }
    if (n_bytes == 0){
        return 0;
    }
    if (buf == NULL){
        return -1;
    }
    // critical section starts
    
    if (buf == NULL){
        return -1;
    }

    for (i = 0; i < n_bytes; i++){
        putc(*(uint8_t *)(buf + i));
    }
    sti();
    // critical section ends
    return i;
}



/*
 * void terminal_switch()
 *      DESCRIPTOR: switch the terminal being displayed.
 *      INTPUT: terminal_num, the number of terminal to switch to 
 *      OUTPUT: none
 *      EFFECT: switch paging for switching terminals and update cursor
 * 
 * 
 */
void 
terminal_switch(uint32_t terminal_num)
{

    set_terminal_page(terminal_num);
    //update location of cursor from terminal struct
    update_cursor(terminals[dis_terminal - 1].cursor_x, terminals[dis_terminal - 1].cursor_y);

    return;
    
    // sti();
}

/*
 * void init_terminal()
 *      DESCRIPTOR: initialze all 3 terminals
 *      INTPUT: none 
 *      OUTPUT: none
 *      EFFECT: initialze data fields of all terminal structs
 * 
 * 
 */
void
init_terminal(){
    uint32_t i;
    for(i = 0; i < 3; i++){
        terminals[i].last_pid = -1;
        terminals[i].kb_num = 0;
        terminals[i].cursor_x = 0;
        terminals[i].cursor_y = 0;
        terminals[i].read_status = 1;
        terminals[i].cur_active = NULL;
    }
}


