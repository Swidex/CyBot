/*
 * ping.c
 *
 *  Created on: Oct 27, 2021
 *      Author: benhall
 */
#include "ping.h"

volatile enum {LOW, HIGH, DONE} state; // set by ISR
volatile unsigned int rising_time; // pulse start time: set by ISR
volatile unsigned int falling_time; // pulse end time: set by ISR

void TIMER3B_Handler();

void ping_init() {
    SYSCTL_RCGCTIMER_R |= 0x8;          // send clock to Timer 3
    SYSCTL_RCGCGPIO_R |= 0x2;           // send clock to port B

    GPIO_PORTB_DIR_R &= ~0x8;           // set as input
    GPIO_PORTB_DEN_R |= 0x8;            // enable digital function
    GPIO_PORTB_AFSEL_R |= 0x8;          // enable alternate function
    GPIO_PORTB_PCTL_R |= 0x7000;        // set PB3 to T3CCP1

    TIMER3_CTL_R &= ~0x100;             // disable TIMER3B during configuration

    timer_waitMillis(5);

    TIMER3_CFG_R = 0x4;                 // split into Timer A & B
    TIMER3_TBMR_R = ( TIMER3_TBMR_R | 0x7 ) & 0x7; // set to capture, edge-time mode
    TIMER3_CTL_R |= 0xC00;              // trigger interrupt on both edges
    TIMER3_TBPR_R = 0xFF;                // set pre-scale value
    TIMER3_TBILR_R = 0xFFFF;            // set pre-scale value

    timer_waitMillis(5);

    TIMER3_CTL_R |= 0x100;              // re-enable TIMER3B

    timer_waitMillis(5);
    NVIC_EN1_R |= 0x10;                 // enable interrupt vector 36;

    timer_waitMillis(5);

    IntRegister(INT_TIMER3B, TIMER3B_Handler);  // bind ISR
    IntMasterEnable();                          // start ISR
}

void send_pulse() {
    state = LOW;                            // set state to low
    TIMER3_CTL_R        &= ~0x100;          // disable timer
    TIMER3_IMR_R        &= ~0x400;          // disable interrupt
    GPIO_PORTB_AFSEL_R  &= ~0x8;            // disable AF
    GPIO_PORTB_DIR_R    |= 0x8;             // set output

    timer_waitMillis(5);                    // WAIT
    GPIO_PORTB_DATA_R   |= 0x8;             // set to low
    GPIO_PORTB_DATA_R   &= ~0x8;            // set to high
    timer_waitMicros(5);                    // signal WAIT
    GPIO_PORTB_DATA_R   |= 0x8;             // set to low
    timer_waitMillis(5);                    // WAIT

    TIMER3_IMR_R        |= 0x400;           // enable interrupt
    TIMER3_ICR_R        |= 0x400;           // clear interrupt
    GPIO_PORTB_AFSEL_R  |= 0x8;             // enable AF
    TIMER3_CTL_R        |= 0x100;           // enable timer
    GPIO_PORTB_DIR_R    &= ~0x8;            // set input
}

float ping_read(){
    send_pulse();
    int timeout = 0;
    while (state != DONE && timeout < 5000) {timeout++;}
    if (timeout >= 5000) { return 1000.0; }
    unsigned int delta = rising_time - falling_time;
    return (float) delta / 969.33;
}

void TIMER3B_Handler() {
    if (TIMER3_MIS_R & 0x400) {
        // check if interrupt has occurred on Timer3B

        if (state == LOW) {
            rising_time = TIMER3_TBR_R;
            state = HIGH;
        }
        else if (state == HIGH) {
            falling_time = TIMER3_TBR_R;
            state = DONE;
        }

        TIMER3_ICR_R |= 0x400; // clear interrupt
    }
}
