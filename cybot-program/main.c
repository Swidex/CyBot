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

int uartTX_bump(int status, oi_t *sensor_data , char uartTX[20]) {
    if (status != -1 && sensor_data->bumpLeft) {
        status = -1;
    } else if (status != 1 && sensor_data->bumpRight) {
        status = 1;
    } else if (status != 0 && !(sensor_data->bumpRight || sensor_data->bumpLeft)){
        status = 0;
    }
    return status;
}

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
    uart_interrupt_init();
    adc_init();
    ping_init();
    servo_init();
    servo_move(90.0);

    oi_t *sensor_data = oi_alloc();
    oi_init(sensor_data);

    bool inAction = false;
    int status = 0;             // var for bumper status
    char uartRX;                // var to hold uart RX data
    char uartTX[20] = "";       // string to hold uart TX data

    lcd_clear();
    lcd_puts("Complete!");

    while(1)
    {
        status = uartTX_bump(status, sensor_data, uartTX); // bumper data
        if (sensor_data->angle != 0 || sensor_data->distance != 0 || status != 0)
        {
            sprintf(uartTX,"UPD,%0.2f,%0.2f,%d\n", sensor_data->angle, sensor_data->distance / 10.0, status);
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
                    oi_setWheels(100, 100);
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
                    oi_setWheels(-100, -100);
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
                    oi_setWheels(100, -100);
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
                    oi_setWheels(-100, 100);
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
