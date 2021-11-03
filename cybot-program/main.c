/**
 * @author Benjamin Hall
 * @date 10/20/2021
 */
#include <stdlib.h>
#include "timer.h"
#include "lcd.h"
#include "open_interface.h"
#include "uart.h"
#include "adc.h"
#include "ping.h"
#include "servo.h"

#define PING_MOD 969.33

void scanInfront(char* uartTX)
{
    lcd_clear();
    sprintf(uartTX, "SCN,%0.0f,%d,%0.2f\n",servo_pos,adc_read(),ping_read() / PING_MOD);
    sendUartString(uartTX);
    lcd_puts(uartTX);
}

void main() {
    lcd_init();
    lcd_clear();
    lcd_puts("Initializing...");

    uart_init(115200);
    adc_init();
    ping_init();
    servo_init();
    servo_move(90.0);

    oi_t *sensor_data = oi_alloc();
    oi_init(sensor_data);

    char uartRX;                // var to hold uart RX data
    char uartTX[20] = "";       // string to hold uart TX data

    lcd_clear();
    lcd_puts("Complete!");

    while(1)
    {
        oi_update(sensor_data);
        oi_setWheels(0, 0);

        uartRX = receive_data; // set local variable
        receive_data = '\0'; // clear RX for interrupts

        switch ( uartRX )
        {
            case 'w': // simple forward
            {
                oi_setWheels(100, 100);

                while (receive_data != 'w') {}
                receive_data = '\0';
                oi_setWheels(0, 0);
				oi_update(sensor_data);
				
                sprintf(uartTX, "MOV,%0.2f\n",sensor_data->distance / 10.0);

                lcd_clear();
                lcd_puts(uartTX);
                sendUartString(uartTX);
                break;
            }
            case 's': // simple back
            {
                oi_setWheels(-100, -100);

                while (receive_data != 's') {}
                receive_data = '\0';
                oi_setWheels(0, 0);

                oi_update(sensor_data);
                sprintf(uartTX, "MOV,%0.2f\n",sensor_data->distance / 10.0);

                lcd_clear();
                lcd_puts(uartTX);
                sendUartString(uartTX);
                break;
            }
            case 'a': // simple left
            {
                oi_setWheels(100, -100);

                while (receive_data != 'a') {}
                receive_data = '\0';
                oi_setWheels(0, 0);

                oi_update(sensor_data);
                sprintf(uartTX, "TRN,%0.2f\n",sensor_data->angle);

                lcd_clear();
                lcd_puts(uartTX);
                sendUartString(uartTX);
                break;
            }
            case 'd': // simple right
            {
                oi_setWheels(-100, 100);

                while (receive_data != 'd') {}
                receive_data = '\0';
                oi_setWheels(0, 0);

                oi_update(sensor_data);
                sprintf(uartTX, "TRN,%0.2f\n",sensor_data->angle);

                lcd_clear();
                lcd_puts(uartTX);
                sendUartString(uartTX);
                break;
            }
            case 'm': // scan
            {
                servo_move(90.0);
                scanInfront(uartTX);
                break;
            }
            case 'n': // scan
            {
                float i;
                for ( i = 0.0; i < 180; i++) {
                    scanInfront(uartTX);
                    servo_move(i);
                }
                break;
            }
        }
    }
}
