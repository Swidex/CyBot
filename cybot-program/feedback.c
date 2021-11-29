/*
 * feedback.c
 *
 *  Created on: Nov 29, 2021
 *      Author: teohys
 */

#include "feedback.h"
#include "lcd.h"

void button_init() {
    SYSCTL_RCGCGPIO_R |=0b00010000;
    GPIO_PORTE_DIR_R &=0xF0;
    GPIO_PORTE_DEN_R |=0x0F;
}

uint8_t button_getButton() {
    if(!(GPIO_PORTE_DATA_R&0b00001000)){
        return 4;
    }
    else if(!(GPIO_PORTE_DATA_R&0b00000100)){
        return 3;
    }
    else if(!(GPIO_PORTE_DATA_R&0b00000010)){
            return 2;
        }
    else if(!(GPIO_PORTE_DATA_R&0b00000001)){
            return 1;
        }
    return 0;
}

char feedback(){
    button_init();
    lcd_init();
    char hello[100];
    int button_num;
    button_num = 0;
    lcd_printf("please rate us for\na rating out of 4!\n");
    while(button_num ==0){
        button_num = button_getButton();
    }
    sprintf(hello,"rated %d out of 4 by our lvoely customer \n\r", button_num);
    lcd_printf(hello);
    return hello;
}

