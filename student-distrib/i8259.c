
/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts
 * are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7 */
uint8_t slave_mask; /* IRQs 8-15 */


/*
Descriptor: this function inits the pic,
both master and slave.
input: none
return: none*/
void
i8259_init(void)
{

	unsigned long flags;
	
    //stop all interrupts and save flags while init
	cli_and_save(flags);

    //mask at initialization.
	master_mask = 0xff;
    slave_mask = 0xff;

    //initialize cmds send to master and slave PIC
    outb(ICW1, MASTER_8259_PORT);
    outb(ICW1, SLAVE_8259_PORT);


    outb(ICW2_MASTER, MASTER_8259_DATA);
    outb(ICW2_SLAVE, SLAVE_8259_DATA);


    outb(ICW3_MASTER, MASTER_8259_DATA);
    outb(ICW3_SLAVE, SLAVE_8259_DATA);

    outb(ICW4, MASTER_8259_DATA);
    outb(ICW4, SLAVE_8259_DATA);


    outb(master_mask, MASTER_8259_DATA);
    outb(slave_mask, SLAVE_8259_DATA);

    //enables irq2, the slave pic
    enable_irq(2);
	restore_flags(flags);

}

/*
Descriptor: this function enables irq in valid
range(0-15)
input: irq num for the irq to be enabled
return: none*/
void
enable_irq(uint32_t irq_num)
{
	unsigned long flags;
	uint8_t new_mask; 
	
	cli_and_save(flags);
	
    uint16_t cur_port;
    if (irq_num > 15) return;
    if (irq_num < 8){
        cur_port = MASTER_8259_DATA;
    }else{
        cur_port = SLAVE_8259_DATA;
        irq_num -= 8;
    }
    new_mask =  inb(cur_port) & ~(1 << irq_num);
    outb(new_mask, cur_port);


	restore_flags(flags);
	
}

/*
Descriptor: this function disables irq in valid
range(0-15)
input: irq num for the irq to be disabled
return: none*/
void
disable_irq(uint32_t irq_num)
{
	unsigned long flags;
	

	uint8_t new_mask; 
    uint16_t cur_port;
    if (irq_num > 15) return;
    if (irq_num < 8){
        cur_port = MASTER_8259_DATA;
    }else{
        cur_port = SLAVE_8259_DATA;
        irq_num -= 8;

    }
    new_mask =  inb(cur_port) | (1 << irq_num);
    outb(new_mask, cur_port);

	restore_flags(flags);	
}

/*
Descriptor: this function sends end of interrupt signal
via cmd port
input:  none
return: none*/
void
send_eoi(uint32_t irq_num)
{
	
	unsigned long flags;
	
	cli_and_save(flags);


	if (irq_num > 15) return;
    if (irq_num < 8){
        outb(EOI | irq_num, MASTER_8259_PORT);
    }else{
        outb(EOI | (irq_num & 7) , SLAVE_8259_PORT);
        outb(EOI | 0x2, MASTER_8259_PORT);
	}

	restore_flags(flags);
}

