#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <inc/tm4c123gh6pm.h>
#include <math.h>

#define BAUD_RATE 115200

// Initialize the UART to communciate between CyBot and PuTTy
void cyBot_uart_init(void){

    SYSCTL_RCGCGPIO_R   |= 0b000010;    //enable clock to GPIO, R1 = port B
    SYSCTL_RCGCUART_R   |= 0b000010;    //enable clock to UART1, R1 = UART1.

    GPIO_PORTB_AFSEL_R  |= 0b00000011;  //enable alternate functions on port b pins 0 and 1
    GPIO_PORTB_PCTL_R   |= 0x00000011;  //enable Rx and Tx on port B on pins 0 and 1 
    GPIO_PORTB_DEN_R    |= 0b00000011;  //set pin 0 and 1 to digital
    GPIO_PORTB_DIR_R    &= 0b11111110;  //set pin 0 to Rx or input
    GPIO_PORTB_DIR_R    |= 0b00000010;  //set pin 1 to Tx or output 

    //calculate baudrate
    double BRD = 16000000 / (16 * BAUD_RATE);
    uint16_t iBRD = trunc(BRD);
    uint16_t fBRD = (int) (BRD - iBRD) * 64 + 0.5;

    UART1_CTL_R &= 0b1111111111111110;  // turn off uart1 while we set it up

    //set baud rate
    UART1_IBRD_R = iBRD;
    UART1_FBRD_R = fBRD;

    UART1_LCRH_R    = 0b01100000;     //set frame, 8 data bits, 1 stop bit, no parity, no FIFO 
    UART1_CC_R      = 0b0000;         //use system clock as source

    UART1_CTL_R |= 0b0000001100000001; //re-enable enable RX, TX, and uart1

}

// Send a byte over the UART from CyBot and PuTTy (Buad Rate 115200, No Parity, No Flow Control)
void cyBot_sendByte(char data){

    while(UART1_FR_R & 0x20) {} // wait until there is room to send data
    UART1_DR_R = data;          // send data

}

// Cybot WAITs to receive a byte from PuTTy (Buad Rate 115200, No Parity, No Flow Control).
// In other words, this is a blocking fucntion.
int cyBot_getByte(void){
    char data = 0; 
    while(UART1_FR_R & UART_FR_RXFE) {} // wait to receive 
    data = (char)(UART1_DR_R & 0xFF);   // mask the 4 error bits and grab only 8 data bits
    return data;
}