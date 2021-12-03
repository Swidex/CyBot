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
    //Config for GPIO Port B Wire 3
//    SYSCTL_RCGCGPIO_R |= 0b10; // clock
//    GPIO_PORTB_DIR_R |=0b1000; // set output
//    GPIO_PORTB_AFSEL_R &= ~0b1000; // not alt func yet
//    GPIO_PORTB_DEN_R |= 0b1000; // turn on digital functions
//    GPIO_PORTB_PCTL_R &= 0xFFFF0FFF; //Alternate function to timer
//    GPIO_PORTB_PCTL_R |= 0x7000; //Alternate function to timer
//
//
//    SYSCTL_RCGCTIMER_R |= 0x8; //Enable Timer3
//    TIMER3_CTL_R &= ~0x100; //Stop counting
//    TIMER3_CFG_R |= 0x4; //Select 16 bit config
//    TIMER3_TBMR_R &= ~0x18; // force zero on what is not a 1
//    TIMER3_TBMR_R |= 0x007; //Capture mode, edge time mode, timer counts up
//
//    TIMER3_CTL_R |= 0xC00; //Both edges on TimerB
//    TIMER3_TBILR_R = 0xFFFF; //Sets upper bound to 65535
//    TIMER3_TBPR_R = 0xFF; //Prescale to 24 bit timer
//
//    TIMER3_ICR_R |= 0x400; //Clear interrupt
//    TIMER3_IMR_R |= 0x00400;
//
//    NVIC_EN1_R |=  0x10;
//    // Remember to bind ISR within TIMER
//    // configuration code
//    IntRegister(INT_TIMER3B, TIMER3B_Handler);

}

void send_pulse() {
    //    state = LOW;                            // set state to low
    //    TIMER3_CTL_R        &= ~0x100;          // disable timer
    //    TIMER3_IMR_R        &= ~0x400;          // disable interrupt
    //    GPIO_PORTB_AFSEL_R  &= ~0x8;            // disable AF
    //    GPIO_PORTB_DIR_R    |= 0x8;             // set output
    //
    //    timer_waitMillis(5);                    // WAIT
    //    GPIO_PORTB_DATA_R   |= 0x8;             // set to low
    //    GPIO_PORTB_DATA_R   &= ~0x8;            // set to high
    //    timer_waitMicros(5);                    // signal WAIT
    //    GPIO_PORTB_DATA_R   |= 0x8;             // set to low
    //    timer_waitMillis(5);                    // WAIT
    //
    //    TIMER3_IMR_R        |= 0x400;           // enable interrupt
    //    TIMER3_ICR_R        |= 0x400;           // clear interrupt
    //    GPIO_PORTB_AFSEL_R  |= 0x8;             // enable AF
    //    TIMER3_CTL_R        |= 0x100;           // enable timer
    //    GPIO_PORTB_DIR_R    &= ~0x8;            // set input

    TIMER3_CTL_R &= ~0x100; //Stop counting
    TIMER3_IMR_R &= ~0x400; //Disable interrupt for CBEIM
    GPIO_PORTB_AFSEL_R &= ~0b1000; //Select software controlled
    GPIO_PORTB_DIR_R |= 0b1000; // Set PB3 as output
    GPIO_PORTB_DATA_R &= ~0b1000;//Set low
    timer_waitMicros(2);
    GPIO_PORTB_DATA_R |= 0b1000;// Set PB3 to high
    // wait at least 5 microseconds based on data sheet
    timer_waitMicros(10);
    GPIO_PORTB_DATA_R &= ~0b1000;// Set PB3 to low
    GPIO_PORTB_DIR_R &= ~0b1000;// Set PB3 as input
    GPIO_PORTB_AFSEL_R |= 0b1000; //Select alternate function
    state = LOW;
    TIMER3_ICR_R |= 0x400; //Clear interrupt
    TIMER3_IMR_R |= 0x400; //Enable interrupt for CBEIM
    TIMER3_CTL_R |= 0x100; //Start counting
}

int num_overflows = 0;

float ping_read(){
        send_pulse();
        int timeout = 0;
        while (state != DONE && timeout < 10000) {timeout++;}
        if (timeout >= 10000) { return 1000.0; }
        unsigned int delta = rising_time - falling_time;
        return (float) delta;

//    send_pulse();
//    while(state != DONE); //Busy wait
//    int time = rising_time - falling_time; //Counting down, so rising is greater than falling
//    if(time < 0) {
//        num_overflows++;
//        time = rising_time - (0xFFFFF + falling_time);
//    }
//    double millis = time / 16000000.0;
//    double centi = (millis / 2) * 34000;
//
//    //lcd_printf("%d", num_overflows);
//    return millis;
}

void TIMER3B_Handler() {
    //    if (TIMER3_MIS_R & 0x400) {
    //        // check if interrupt has occurred on Timer3B
    //
    //        if (state == LOW) {
    //            rising_time = TIMER3_TBR_R;
    //            state = HIGH;
    //        }
    //        else if (state == HIGH) {
    //            falling_time = TIMER3_TBR_R;
    //            state = DONE;
    //        }
    //
    //        TIMER3_ICR_R |= 0x400; // clear interrupt
    //    }

    if((TIMER3_MIS_R & 0x400) == 0)
        return; //Interrupt was not capture event

    TIMER3_ICR_R |= 0x400; //Clear interrupt

    if(state == LOW) {
        rising_time = TIMER3_TBR_R;
        state = HIGH;
    } else if (state == HIGH) {
        falling_time = TIMER3_TBR_R;
        state = DONE;
    }
}
