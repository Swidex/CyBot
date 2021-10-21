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
#include "cyBot_Scan.h"

#define MAX_OBJECTS 10
#define MAX_DIST 800
#define SCAN_INTERVAL 2

void scanInfront(char uartTX[20]){
    int theta;
    cyBOT_Scan_t scanData;
    for (theta = 0; theta <= 180; theta += 2 ) {
        cyBOT_Scan(theta, &scanData); // scan at theta degrees.
        sprintf(uartTX, "SCN,%d,%d,%0.2f\n",theta,adc_read(),scanData.sound_dist);
        sendUartString(uartTX);
    }
}

int bumpStatus(int status, oi_t *sensor_data , char uartTX[20]) {
    if (status != 1 && sensor_data->bumpLeft) {
        sendUartString("BMP,-1\n");
        status = 1;
    } else if (status != 2 && sensor_data->bumpRight) {
        sendUartString("BMP,1\n");
        status = 2;
    } else if (status > 0 && !(sensor_data->bumpRight || sensor_data->bumpLeft)){
        sendUartString("BMP,0\n");
        status = 0;
    }
    return status;
}

int uartRX_int(int maxTimeout, char endChar) {
    int i = 0;
    int timeout = 0;
    char stringRX[10];

    while (timeout <= maxTimeout) {
        char uartRX = receive_data;
        receive_data = '\0';
        timer_waitMillis(10);
        if ( uartRX == endChar ) {
            int intRX = atoi(stringRX);
            return intRX;
        } else if ( uartRX != '\0'){
            stringRX[i] = uartRX;
            i++;
        } else {
            timeout++;
        }
    }
}

void main() {
    lcd_init();
    uart_init(115200);
    uart_interrupt_init();
    adc_init();
    oi_t *sensor_data = oi_alloc();
    oi_init(sensor_data);
    cyBOT_init_Scan();
    lcd_clear();

    // calibration for servo
    left_calibration_value = 1272250;
    right_calibration_value = 301000;

    int status = 0;
    char uartRX;
    char uartTX[20] = "";

    while(1)
    {
        oi_update(sensor_data);
        oi_setWheels(0, 0); // stop
        status = bumpStatus(status, sensor_data, uartTX); // bumper data
        uartRX = receive_data; // uart data
        receive_data = '\0'; // clear global var
        switch ( uartRX )
        {
            case 'w': // forward and back
            {
                int dist = uartRX_int(50, 'x');
                move(sensor_data, dist);
                break;
            }
            case 'd': // turn left and right
            {
                int deg = uartRX_int(50, 'x');
                turn(sensor_data, deg);
                break;
            }
            case 'm': //scan
            {
                scanInfront(uartTX);
                break;
            }
        }
    }
}
