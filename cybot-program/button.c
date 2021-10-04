/*
 * button.c
 *
 *  Created on: Jul 18, 2016
 *      Author: Eric Middleton, Zhao Zhang, Chad Nelson, & Zachary Glanz.
 *
 *  @edit: Lindsey Sleeth and Sam Stifter on 02/04/2019
 *  @edit: Phillip Jone 05/30/2019: Mearged Spring 2019 version with Fall 2018
 */
 


//The buttons are on PORTE 3:0
// GPIO_PORTE_DATA_R -- Name of the memory mapped register for GPIO Port E, 
// which is connected to the push buttons
#include "button.h"

volatile int button_event;
volatile int button_num;

void button_init() {
	static uint8_t initialized = 0;

	if(initialized){
		return;
	}

	SYSCTL_RCGCGPIO_R |= 0b010000;
	GPIO_PORTE_DIR_R &= 0b11110000;
	GPIO_PORTE_DEN_R |= 0b00001111;
	initialized = 1;
}

/**
 * Initialize and configure PORTE interupts
 */
void init_button_interrupts() {

    // mask bits we are using
    GPIO_PORTE_IM_R &= 0b11110000;

    // use edge sensing
    GPIO_PORTE_IS_R &= 0b11110000;

    // use both edges for detecting when a button is released and pressed
    GPIO_PORTE_IBE_R |= 0b00001111;

    // clear interrupts
    GPIO_PORTE_ICR_R = 0b11111111;

    // unmask bits we are using
    GPIO_PORTE_IM_R |= 0b00001111;

    // initialize interrupts
    NVIC_EN0_R |= 0b000010000;

    // Bind the interrupt to the handler.
    IntRegister(INT_GPIOE, gpioe_handler);
}

/**
 * Interrupt handler -- executes when a GPIO PortE hardware event occurs (i.e., for this lab a button is pressed)
 */
void gpioe_handler() {
    // clear interrupts
    GPIO_PORTE_ICR_R = 0b11111111;

    // set button num to getButton();
    button_num = button_getButton();
}

/**
 * Returns the position of the leftmost button being pushed.
 * @return the position of the leftmost button being pushed. 4 is the leftmost button, 1 is the rightmost button.  0 indicates no button being pressed
 */
uint8_t button_getButton(void) {
    if ( ((GPIO_PORTE_DATA_R >> 3) & 1) == 0 )
    {
        return 4;
    } else if ( ((GPIO_PORTE_DATA_R >> 2) & 1) == 0 )
    {
        return 3;
    } else if ( ((GPIO_PORTE_DATA_R >> 1) & 1) == 0 )
    {
        return 2;
    } else if ( ((GPIO_PORTE_DATA_R >> 0) & 1) == 0 )
    {
        return 1;
    }
    return 0;
}





