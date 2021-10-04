/**
 * lab5_template.c
 * 
 * Template file for CprE 288 lab 5
 *
 * @author Zhao Zhang, Chad Nelson, Zachary Glanz
 * @date 08/14/2016
 *
 * @author Phillip Jones, updated 6/4/2019
 */

#include "button.h"
#include "timer.h"
#include "lcd.h"
#include "open_interface.h"
#include "movement.h"

#include "cyBot_uart.h"
#include "cyBot_Scan.h"

// Defined in button.c : Used to communicate information between the
// the interupt handeler and main.
extern volatile int button_event;
extern volatile int button_num;
INFRONT_ANGLE = 110;

void sendUartString( char msg[] ) {
    int i = 0;
    while ( msg[i] != '\0' ){
        cyBot_sendByte(msg[i]);
        i++;
    }
}

double convertIRToDist( int ir_val ) {
    return 6410000 * pow(ir_val,-1.69);
}

int main(void) {
	button_init();
	lcd_init();
	init_button_interrupts();
	oi_t *sensor_data = oi_alloc();
    oi_init(sensor_data);

	cyBot_uart_init_clean();  // Clean UART initialization, before running your UART GPIO init code

	// Complete this code for configuring the  (GPIO) part of UART initialization
    SYSCTL_RCGCGPIO_R |= 0b000010;
    timer_waitMillis(1);            // Small delay before accessing device after turning on clock
    GPIO_PORTB_AFSEL_R |= 0x03;
    GPIO_PORTB_PCTL_R &= 0x0011;      // Force 0's in the desired locations
    GPIO_PORTB_PCTL_R |= 0x0011;     // Force 1's in the desired locations
    GPIO_PORTB_DEN_R |= 0x03;
    GPIO_PORTB_DIR_R &= 0x01;      // Force 0's in the desired locations
    GPIO_PORTB_DIR_R |= 0x01;      // Force 1's in the desired locations

    cyBot_uart_init_last_half();  // Complete the UART device initialization part of configuration
	
    //Initialize scan sensors
    cyBOT_init_Scan();
    cyBOT_Scan_t scanData;
    char returnString[30];

    while(1)
    {
        switch ( button_num )
        {
        case 1:
            lcd_clear();
            lcd_puts("S1");
            sendUartString("You pressed button 1!\n\r");
            break;
        case 2:
            lcd_clear();
            int theta;
            for (theta = 0; theta <= 180; theta += 2 ) {
                cyBOT_Scan(theta, &scanData); // scan at theta degrees.
                timer_waitMillis(100); // wait so it doesn't break
                sprintf(returnString, "%d,%0.2f\n",theta,(convertIRToDist(scanData.IR_raw_val) + scanData.sound_dist) / 2.0);
                sendUartString(returnString);
            }
            sendUartString("Complete\n");
            break;
        case 3:
            lcd_clear();
            cyBOT_Scan(INFRONT_ANGLE, &scanData);
            double calcDist = convertIRToDist(scanData.IR_raw_val);
            sprintf(returnString, "%d %0.2f",scanData.IR_raw_val, (calcDist+scanData.sound_dist) / 2.0);
            printf("%0.2f\t%02.f\n", calcDist, scanData.sound_dist);
            lcd_puts(returnString);
            break;
        case 4:
            lcd_clear();
            lcd_puts("Calibrating...");
            float dist = 100.0;
            while (dist > 10.0)
            {
                cyBOT_Scan(INFRONT_ANGLE, &scanData);
                dist = scanData.sound_dist;
                sprintf(returnString, "%0.0f,%d\n", dist, scanData.IR_raw_val);
                sendUartString(returnString);
                move(sensor_data,5);
                if (button_num == 3) {
                    lcd_clear();
                    oi_setWheels(0,0);
                    lcd_puts("Calibration canceled.");
                    break;
                }
            }
            break;
        }
    }
}
