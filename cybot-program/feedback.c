/*
 * feedback.c
 *
 *  Created on: Nov 29, 2021
 *      Author: teohys
 */

#include "feedback.h"
#include "lcd.h"
#include "button.h"


char* feedback(){
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

