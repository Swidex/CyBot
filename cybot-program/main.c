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
#include "Sound.h"
#include "cliff.h"

#define PING_MOD 969.33

void scanInfront(char* uartTX)
{
    lcd_clear();
    sprintf(uartTX, "1,%0.0f,%d,%0.2f\n",servo_pos,adc_read(),ping_read() / PING_MOD);
    sendUartString(uartTX);
    lcd_puts(uartTX);
}

void main() {
    lcd_init();
    lcd_clear();
    lcd_puts("Initializing...");

    uart_init(115200);
    uart_interrupt_init();
    adc_init();
    ping_init();
    servo_init();
    servo_move(90.0);

    oi_t *sensor_data = oi_alloc();
    oi_init(sensor_data);

    unsigned int lastCliffData = 0b00000000;

    int lBumpLast = 0;
    int rBumpLast = 0;
    bool inAction = false;
    char uartRX;                // var to hold uart RX data
    char uartTX[100] = "";       // string to hold uart TX data

    lcd_clear();
    lcd_puts("Complete!");

    while(1)
    {

        unsigned int cliffData = updateCliffStatus(sensor_data);
        if (cliffData != lastCliffData && cliffData != 0b00000000) {
            oi_setWheels(-50, -50);
            play_sound(2);
            timer_waitMillis(50);
            oi_setWheels(0, 0);
            lastCliffData = cliffData;
            sprintf(uartTX,"2,%d", cliffData);
            sendUartString(uartTX);
        }

        if (sensor_data->angle != 0 ||
            sensor_data->distance != 0 ||
            sensor_data->bumpLeft != lBumpLast ||
            sensor_data->bumpRight != rBumpLast
         )
        {
            lBumpLast = sensor_data->bumpLeft;
            rBumpLast = sensor_data->bumpRight;
            sprintf(uartTX,"0,%0.2f,%0.2f,%d,%d\n",
                sensor_data->angle,
                sensor_data->distance / 10.0,
                sensor_data->bumpLeft,
                sensor_data->bumpRight
            );
            sendUartString(uartTX);
        }

        oi_update(sensor_data);
        uartRX = receive_data; // set local variable
        receive_data = '\0'; // clear RX for interrupts

        switch ( uartRX )
        {
            case 'w': // simple forward
            {
                if (inAction)
                {
                    oi_setWheels(0, 0);
                    inAction = false;
                } else
                {
                    oi_setWheels(50, 50);
                    inAction = true;
                }
                break;
            }
            case 's': // simple back
            {
                if (inAction)
                {
                    oi_setWheels(0, 0);
                    inAction = false;
                } else
                {
                    oi_setWheels(-50, -50);
                    inAction = true;
                }
                break;
            }
            case 'a': // simple left
            {
                if (inAction)
                {
                    oi_setWheels(0, 0);
                    inAction = false;
                } else
                {
                    oi_setWheels(50, -50);
                    inAction = true;
                }
                break;
            }
            case 'd': // simple right
            {
                if (inAction)
                {
                    oi_setWheels(0, 0);
                    inAction = false;
                } else
                {
                    oi_setWheels(-50, 50);
                    inAction = true;
                }
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
