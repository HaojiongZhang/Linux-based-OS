//added by lhy 14/10/2022 19:01
#include "lib.h"
#include "rtc_handler.h"
#include "i8259.h"

static int read_lock = 0;
static int glob_rate[3];
static int counter[3] = {0, 0, 0};
/*
Descriptor: this function is used to initialize the RTC interrupt function
input : NONE
return  : NONE*/
void init_rtc()
{
    // Initialization code, from https://wiki.osdev.org/RTC
    outb(REG_B, RTC_PORT);	  
    char prev1 = inb(RTC_DATA);	     
    outb(REG_B, RTC_PORT);	   
    outb(prev1 | 0x40, RTC_DATA);  
    set_rtc_freq(FREQ_1024);
    enable_irq(8);
}
/*
Descriptor: this function is used to set the different frequency to the rtc
input: unsigned int freq
return: 1 for success*/

int set_rtc_freq(uint32_t freq)
{
    /*Refrence from: https://wiki.osdev.org/RTC */
    outb(REG_A, RTC_PORT);		                    
    char prev2= inb(RTC_DATA);	                    
    outb(REG_A, RTC_PORT);		                    
    outb((prev2 & 0xF0) | freq, RTC_DATA);          
    return 1;
}

/*Descriptor: this is the interrupt handler function, which is not derictly called
but is used in the wrapper function
input: NONE
return: NONE*/
void RTC_interrupt()
{
    
    /*Refrence from: https://wiki.osdev.org/RTC */
    read_lock = 1;
    counter[sche_terminal]++;
    outb(0x0C, RTC_PORT);	                        
    inb(RTC_DATA);		                            
    // printf("s");
    send_eoi(0x08);
 
}
/*
Descriptor: this is the rtc_read function, which is used to read the frequency of the rtc
once the interrupt happens
Input: ignored
return: 0*/
int32_t rtc_read (int32_t fd, void* buf, int32_t nbytes)
{
    // while(!read_lock){};            //when no interrupt happens, lock the function
    // read_lock = 0;                  //once get it, set it to 0 then return 0
    while(counter[sche_terminal] <= glob_rate[sche_terminal] / 6){};
    counter[sche_terminal] = 0;
    return SUCCESS; 
}
/*Descriptor: this is the rtc_write function, which is used to set the frequency to the RTC
the frequency is passed through a buffer, and the size of the number is fixed to 4 bytes
input: ignored fd, const uint32_t* buf, int32_t nbytes
return: 0/-1*/
int32_t rtc_write (int32_t fd, void* buf, int32_t nbytes)
{
    cli();
    int freq, freq_set;

    if(nbytes != 4) return RTC_FAIL;                //if the freq is not a int, return fail
    if(buf == NULL) return RTC_FAIL;                //if buffer is empty, return fail
    freq = *(int*)buf;
    
    if(check_pow2(freq) == -1) return RTC_FAIL;     //check if the frequency is power of 2
    glob_rate[sche_terminal] = 1024 / freq;
    freq_set = get_freq(freq);                      //get the index of frequency
    set_rtc_freq(0x06);                         //set the frequency of rtc
    counter[0] = 0;
    counter[1] = 0;
    counter[2] = 0;
    sti();
    
    return SUCCESS;
    
}
/*Descriptor: this is the rtc_open function, which is used to open the rtc and set
the frequency to 2 Hz.
input: ignored
return: 0*/
int32_t rtc_open (const uint8_t* filename)
{
    int freq;
    freq = get_freq(2);                             //set the default freq to 2
    if(set_rtc_freq(freq)) return SUCCESS;          //return
    return RTC_FAIL;
}
/*Descriptor: this is the rtc_close function, nothing useful now
input: ignored
return: 0*/
int32_t rtc_close (int32_t fd)
{
    return SUCCESS;                                 //always return success
}
/*Descriptor: this function is used to get the index of frequency from 0x06 to 0x0F
input: int freq
return: int freq_index*/
int32_t get_freq(int freq)
{
    int index_freq;
    index_freq = log_2((1 << 15) / freq) + 1;       //according to the computation, get the index of the frequency
    return index_freq;
}
/*this function is used to check if the function is the power of 2
input: int freq
return: 0/-1*/
int32_t check_pow2(int freq)
{
    int i;
    if(freq > 1024 || freq < 1) return RTC_FAIL;    //if freq is not in the range 
    while(freq > 1){    
        i = freq % 2;
        if(i) return RTC_FAIL;                      //check the modulo to 2 
        freq /= 2;                                  //check if the int is a power of 2
    }
    return SUCCESS;
}
/*this function i used to compute the log index of 2
input: int base
return: index i*/
int log_2(int base)
{
    int i;
    for(i = 0; i < base; i++){                      //calculate the log 2 of the number
        if((1 << i) == base) break;
    }
    return i;
}
