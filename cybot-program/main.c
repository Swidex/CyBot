/**
 * @author Benjamin Hall
 * @date 10/20/2021
 */
#include <stdlib.h>
#include "timer.h"
#include "lcd.h"
#include "open_interface.h"
#include "movement.h"
#include "uart.h"
#include "adc.h"
#include "ping.h"

#define MAX_OBJECTS 10
#define MAX_DIST 800
#define SCAN_INTERVAL 2
#define PING_MOD 969.33

int uartTX_bump(int status, oi_t *sensor_data , char uartTX[20]) {
    /*if (status != 1 && sensor_data->bumpLeft) {
        sendUartString("BMP,-1\n");
        status = 1;
    } else if (status != 2 && sensor_data->bumpRight) {
        sendUartString("BMP,1\n");
        status = 2;
    } else if (status > 0 && !(sensor_data->bumpRight || sensor_data->bumpLeft)){
        sendUartString("BMP,0\n");
        status = 0;
    }*/
    return status;
}

void main() {
    lcd_init();

    lcd_clear();
    lcd_puts("Initializing...");

    uart_init(115200);
    adc_init();
    ping_init();
    oi_t *sensor_data = oi_alloc();
    oi_init(sensor_data);

    int status = 0;             // var for bumper status
    char uartRX;                // var to hold uart RX data
    char uartTX[20] = "";       // string to hold uart TX data

    lcd_clear();
    lcd_puts("Complete!");

    while(1)
    {
        oi_update(sensor_data);
        oi_setWheels(0, 0);

        status = uartTX_bump(status, sensor_data, uartTX); // bumper data
        uartRX = receive_data; // set local variable
        receive_data = '\0'; // clear RX for interrupts

        switch ( uartRX )
        {
            case 'w': // simple forward
            {
                oi_setWheels(100, 100);

                while (receive_data != 'w') {
                    printf("%d,%d\n",sensor_data->requestedLeftVelocity, sensor_data->requestedRightVelocity);
                }
                oi_update(sensor_data);
                receive_data = '\0';
                oi_setWheels(0, 0);

                sprintf(uartTX, "MOV,%0.2f\n",sensor_data->distance / 10.0);

                lcd_clear();
                lcd_puts(uartTX);
                sendUartString(uartTX);
                break;
            }
            case 's': // simple back
            {
                oi_setWheels(-100, -100);

                while (receive_data != 's') { }
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
                lcd_clear();
                sprintf(uartTX, "SCN,%d,%0.2f\n",adc_read(),ping_read() / PING_MOD);
                sendUartString(uartTX);
                lcd_puts(uartTX);
                break;
            }
        }
    }
}
