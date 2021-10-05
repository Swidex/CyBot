/**
 * main.c
 *
 * @author Benjamin Hall
 * @date 10/4/2021
 *
 */

#include "button.h"
#include "timer.h"
#include "lcd.h"
#include "open_interface.h"
#include "movement.h"
#include "cyBot_uart.h"
#include "cyBot_Scan.h"

#define INFRONT_ANGLE = 110 // different for every CyBot

// Defined in button.c : Used to communicate information between the
// the interupt handeler and main.
extern volatile int button_event;
extern volatile int button_num;

void sendUartString( char msg[] ) {
    // send string to UART
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

    // initialize sensors
	oi_t *sensor_data = oi_alloc();
    oi_init(sensor_data);

	cyBot_uart_init_clean();  // clean UART initialization

	// initialize GPIO for UART
    SYSCTL_RCGCGPIO_R |= 0b000010;
    timer_waitMillis(1);    // Small delay before accessing device after turning on clock
    GPIO_PORTB_AFSEL_R |= 0x03;
    GPIO_PORTB_PCTL_R &= 0x0011;
    GPIO_PORTB_PCTL_R |= 0x0011;
    GPIO_PORTB_DEN_R |= 0x03;
    GPIO_PORTB_DIR_R &= 0x01;
    GPIO_PORTB_DIR_R |= 0x01;

    cyBot_uart_init_last_half(); // UART device initalization completion
	
    // initialize scan sensors
    cyBOT_init_Scan();
    cyBOT_Scan_t scanData;

    char returnString[30];

    while(1)
    {
        char cmd = cyBot_getByte();
        switch ( cmd )
        {
        case 'w': //move forward
            double dist = move(sensor_data, 10);

            // send back distance for accuracy
            sprintf(returnString,"%0.2f\n",dist);
            sendUartString(returnString);

            break;
        case 's': //move back
            double dist = move(sensor_data, -10);

            // send back distance for accuracy
            sprintf(returnString,"%0.2f\n",dist);
            sendUartString(returnString);

            break;
        case 'd': //turn right
            double angle = turn(sensor_data, -12);

            // send back angle for accuracy
            sprintf(returnString,"%d\n",angle);
            sendUartString(returnString);

            break;
        case 'a': //turn left
            double angle = turn(sensor_data, 12);

            // send back angle for accuracy
            sprintf(returnString,"%d\n",angle);
            sendUartString(returnString);

            break;
        case 'm': //scan
            lcd_clear();
            lcd_puts("Scanning");

            int theta;
            for (theta = 0; theta <= 180; theta += 2 ) {
                cyBOT_Scan(theta, &scanData); // scan at theta degrees.
                timer_waitMillis(100); // wait so it doesn't break
                sprintf(returnString, "%d,%0.2f,%0.2f\n",theta,convertIRToDist(scanData.IR_raw_val),scanData.sound_dist);
                sendUartString(returnString);
            }
            sendUartString("Complete\n"); // tell uart we are done

            break;
        case 'c': // Calibrate IR sensor to real distances
            lcd_clear();
            lcd_puts("Calibrating");

            float dist = 100.0;
            while (dist > 10.0) // continue until distance is less than 10
            {
                // take scan directly infront of CyBot
                cyBOT_Scan(INFRONT_ANGLE, &scanData);
                dist = scanData.sound_dist;

                if (dist > 100) { continue; } // skip if distance is greater due to scanner inaccuracies

                sprintf(returnString, "%0.0f,%d\n", dist, scanData.IR_raw_val);
                sendUartString(returnString);

                // move forward for next scan
                move(sensor_data,5);

                if (button_num == 4) {
                    // stop calibration if button 4 is pressed
                    lcd_clear();
                    oi_setWheels(0,0);
                    lcd_puts("Calibration canceled.");
                    timer_waitMillis(1000); // wait one second before taking input
                    break;
                }
            }
            sendUartString("Complete\n") // tell uart we are done calibrating
            break;
        }
    }
    oi_free(sensor_data);
}
