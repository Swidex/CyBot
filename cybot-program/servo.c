#include "servo.h"

volatile float servo_pos;

void servo_init()
{
    SYSCTL_RCGCGPIO_R |= 0x2;                   // send clock on port B
    SYSCTL_RCGCTIMER_R |= 0x2;                  // send clock on TIMER1

    timer_waitMillis(5);

    GPIO_PORTB_DIR_R |= 0x20;                   // set PB5 as output
    GPIO_PORTB_DEN_R |= 0x20;                   // set PB5 as digital
    GPIO_PORTB_AFSEL_R |= 0x20;                 // use alt func on PB5
    GPIO_PORTB_PCTL_R |= 0x00700000;            // pctl

    TIMER1_CTL_R &= ~0x100;                      // disable TIMER1B
    TIMER1_CFG_R |= 0x4;                        // set to 16bit
    TIMER1_TBMR_R |= 0xA;                        // set to periodic

    TIMER1_TBPR_R = 320000 >> 16;               // set timer prescaler
    TIMER1_TBILR_R = 320000;                    // set period
    servo_pos = 0;

    TIMER1_CTL_R |= 0x100;                      // enable TIMER1B
}

void servo_move(float degrees)
{
    servo_pos = degrees;
    TIMER1_CTL_R &= ~0x100;                  // disable TIMER1B

    int match_value = ( (0.0095*(degrees) + .5) / 0.0000625);

    TIMER1_TBMATCHR_R = ( 320000 - match_value ) & 0xFFFF;
    TIMER1_TBPMR_R = ( 320000 - match_value) >> 16;

    TIMER1_CTL_R |= 0x100;                  // enable TIMER1B
}
