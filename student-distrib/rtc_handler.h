#ifndef RTC_HANDLER_H
#define RTC_HANDLER_H 
/*the register number, which is referenced from OSdev.com*/
#define     REG_B       0x8B
#define     REG_A       0x8A
#define     RTC_PORT    0x70
#define     RTC_DATA    0x71
#define     FREQ_1024   0x06
#define     REG_C       0x0C
#define     SUCCESS     0
#define     RTC_FAIL        -1

#ifndef ASM
extern void init_rtc();

extern int set_rtc_freq(uint32_t freq);

extern void RTC_interrupt();

extern void RTC_handler();

extern int32_t rtc_read (int32_t fd, void* buf, int32_t nbytes);

extern int32_t rtc_write (int32_t fd, void* buf, int32_t nbytes);

extern int32_t rtc_open (const uint8_t* filename);

extern int32_t rtc_close (int32_t fd);

int32_t get_freq(int freq);

int32_t check_pow2(int freq);

int log_2(int base);

#endif
#endif
