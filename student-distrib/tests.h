#ifndef TESTS_H
#define TESTS_H
#include "lib.h"


// test launcher
void launch_tests();
void divide_by_zero();

void deref_invalid_address_1();
void deref_invalid_address_2();
void deref_invalid_address_3();
void deref_invalid_address_4();
void deref_null();
int irq_test();
int rtc_test();

int deref_valid_address();

//checkpoint 2 test

int open_fileName();
void print_buf(uint8_t* buf, uint32_t length);
int invalid_null_buffer();

#endif /* TESTS_H */
