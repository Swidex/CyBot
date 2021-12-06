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

#define CYBOT_3

#ifdef CYBOT_3
    #define PING_MOD 969.33
    #define SERVO_CALIBRATION -30
    #define SERVO_COEFFICIENT 1.17
    #define CLIFF_MIN 750
    #define CLIFF_MAX 2500
#endif

#ifdef CYBOT_9
    #define PING_MOD 969.33
    #define SERVO_CALIBRATION -18

    #define CLIFF_MIN 1000
    #define CLIFF_MAX 2700
#endif

extern volatile bool inAction;
bool isFast = false;
int speed = 50;

void scanInfront(char* uartTX)
{
    lcd_clear();

    double avg_val;
    int j;
    for(j=0; j<16; j++)
    {
        avg_val += adc_read();
        timer_waitMillis(1);
    }

    avg_val /= 16;

    double ping_val = ping_read();

    sprintf(uartTX, "1,%0.0f,%f,%0.2f\n",servo_pos,avg_val,ping_val / PING_MOD);
    sendUartString(uartTX);
    lcd_puts(uartTX);
}

void calibrateMotors(oi_t *sensor) {
    int leftEncoderVal = sensor->leftEncoderCount;
    int rightEncoderVal = sensor->rightEncoderCount;

    oi_setWheels(100, 100);
    timer_waitMillis(500);
    oi_setWheels(0, 0);

    oi_update(sensor);

    leftEncoderVal -= sensor->leftEncoderCount;
    rightEncoderVal -= sensor->rightEncoderCount;

    printf("%f %f", 1.0, (leftEncoderVal / rightEncoderVal));
    oi_setMotorCalibration(1.0,  (((float) leftEncoderVal) / rightEncoderVal) );
}

void main() {
    timer_init();
    lcd_init();
    lcd_clear();
    lcd_puts("Initializing...");

    uart_init(115200);
    uart_interrupt_init();
    adc_init();
    ping_init();
    servo_init();
    servo_set_calibration(SERVO_CALIBRATION, SERVO_COEFFICIENT);
    cliff_set_calibration(CLIFF_MIN, CLIFF_MAX);
    servo_move(90.0);

    oi_t *sensor_data = oi_alloc();
    oi_init(sensor_data);

    unsigned int lastCliffData = 0b00000000;

    //bool inAction = false;
    char uartRX;                // var to hold uart RX data
    char uartTX[50] = "";       // string to hold uart TX data

    oi_setWheels(0, 0);
    lcd_clear();
    lcd_puts("Calibrating...");

    calibrateMotors(sensor_data);

    lcd_clear();
    lcd_puts("Ready!");

    while(1)
    {

        if (sensor_data->angle != 0 || sensor_data->distance != 0)
        {
            sprintf(uartTX,"0,%0.2f,%0.2f\n", sensor_data->angle, sensor_data->distance / 10.0);
            sendUartString(uartTX);
        }

        unsigned int cliffData = updateCliffStatus(sensor_data);
        if (cliffData != lastCliffData && cliffData != 0b00000000) {
            oi_setWheels(-50, -50);
            play_sound(2);
            timer_waitMillis(100);
            oi_setWheels(0, 0);
            lastCliffData = cliffData;
            sprintf(uartTX,"2,%d\n", cliffData);
            sendUartString(uartTX);

            oi_update(sensor_data);

            sprintf(uartTX,"0,%0.2f,%0.2f\n", sensor_data->angle, sensor_data->distance / 10.0);
            sendUartString(uartTX);
        }

        if (sensor_data->bumpLeft == 1 || sensor_data->bumpRight == 1) {
            sprintf(uartTX,"3,%d,%d\n",sensor_data->bumpLeft,sensor_data->bumpRight);
            sendUartString(uartTX);

            oi_setWheels(-50, -50);
            play_sound(2);
            timer_waitMillis(1000);
            oi_setWheels(0, 0);

            oi_update(sensor_data);

            sprintf(uartTX,"3,%d,%d\n",sensor_data->bumpLeft,sensor_data->bumpRight);
            sendUartString(uartTX);

            sprintf(uartTX,"0,%0.2f,%0.2f\n", sensor_data->angle, sensor_data->distance / 10.0);
            sendUartString(uartTX);
        }

        oi_update(sensor_data);
        uartRX = receive_data; // set local variable
        receive_data = '\0'; // clear RX for interrupts


        lcd_printf("%d %d %d %d", sensor_data->cliffFrontLeftSignal, sensor_data->cliffFrontRightSignal, sensor_data->cliffLeftSignal, sensor_data->cliffRightSignal);

        switch ( uartRX )
        {
        case 'w': // simple forward
        {
            if (inAction)
            {
                oi_setWheels(0, 0);
                //inAction = false;
            } else
            {
                oi_setWheels(speed, speed);
                //inAction = true;
            }
            break;
        }
        case 's': // simple back
        {
            if (inAction)
            {
                oi_setWheels(0, 0);
                //inAction = false;
            } else
            {
                oi_setWheels(-speed, -speed);
                //inAction = true;
            }
            break;
        }
        case 'a': // simple left
        {
            if (inAction)
            {
                oi_setWheels(0, 0);
                //inAction = false;
            } else
            {
                oi_setWheels(speed, -speed);
                //inAction = true;
            }
            break;
        }
        case 'd': // simple right
        {
            if (inAction)
            {
                oi_setWheels(0, 0);
                //inAction = false;
            } else
            {
                oi_setWheels(-speed, speed);
                //inAction = true;
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
            servo_move(0);
            timer_waitMillis(750);
            int i;
            for ( i = 0; i < 180; i++) {
                scanInfront(uartTX);
                servo_move(i);
                //timer_waitMillis(40);
            }
            break;
        }
        case 'p': // change speed
        {
            isFast = !isFast;
            if(isFast) {
                speed = 150;
            } else {
                speed = 50;
            }
            break;
        }
        }
    }
}
