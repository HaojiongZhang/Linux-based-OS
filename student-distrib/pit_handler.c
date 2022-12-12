#include "pit_handler.h"
#include "scheduler.h"


/*
 *   pit_init
 *   DESCRIPTION: initialize the PIT to the correct channel and models as well as initializing the interrupt frequency
 *   INPUTS: NONE
 *   RETURN VALUE: NONE
 * 			
 */

void pit_init(){
    unsigned long flags;
    cli_and_save(flags);

    outb(LOHIBYTE, PIT_COMMAND_PORT);   //set mode to lobyte/hibyte
    //outb(SQUAREWAVE, PIT_COMMAND_PORT); //set mode to squarewave

    outb(COUNTER&0xFF, PIT_DATA_PORT);      //low 8 bits
    outb(COUNTER>>8, PIT_DATA_PORT);   //high 8 bits

    enable_irq(0);

	restore_flags(flags);
    
}

/*
 *   pit_interrupt
 *   DESCRIPTION: signals the scheduler to run
 *   INPUTS: NONE
 *   RETURN VALUE: NONE 
 * 			
 */
void pit_interrupt(){
    scheduler(); 
}
