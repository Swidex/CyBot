/// Simple 'Hello, world' program
/**
 * This program prints "Hello, world" to the LCD screen
 * @author Chad Nelson
 * @date 06/26/2012
 *
 * updated: phjones 9/3/2019
 * Describtion: Added timer_init call, and including Timer.h
 */

#include "timer.h"
#include "lcd.h"
#include "open_interface.h"
#include "movement.h"
#include "uart.h"
#include "cyBot_Scan.h"

#define MAX_OBJECTS 10
#define MAX_DIST 800
#define SCAN_INTERVAL 2

float calculateActualWidth(int ai, int ae, float h){
    // ai = initial angle
    // ae = end angle
    // h = hypotenuse
    return (2*h) * cos((90 - (( (float) ae - ai) /2 )) * (M_PI / 180.0) );
}

double convertIRToDist( int ir_val ) {
    return 2600000 * pow(ir_val,-1.56);
}

int scanInfront(int *startTheta,int *endTheta, float *hypotenuse, int offset, int initCounter){
    int objCounter = initCounter;
    int theta;
    char returnString[30];
    cyBOT_Scan_t scanData;

    for (theta = 0; theta <= 180; theta += SCAN_INTERVAL ) {
        cyBOT_Scan(theta, &scanData); // scan at theta degrees.
        timer_waitMillis(100); // wait so it doesn't break
        sprintf(returnString, "SCN,%d,%0.2f,%0.2f\n",theta,convertIRToDist(scanData.IR_raw_val),scanData.sound_dist);
        sendUartString(returnString);

        // check if we've stored a value for the object
        if (startTheta[objCounter] != 0) {
            //check to see if current distance is near previous distance
            if (convertIRToDist(scanData.IR_raw_val) > 100) {
                //new distance is too far away, iterate objectCounter and log distance
                endTheta[objCounter] = theta - SCAN_INTERVAL + offset;
                objCounter++;
            }
        } else if (scanData.IR_raw_val > MAX_DIST) {
            // start new object
            startTheta[objCounter] = theta + offset;
            hypotenuse[objCounter] = scanData.sound_dist;
        }
        sendUartString(returnString);
    }
    return objCounter;
}

void main() {
    lcd_init();
    uart_init(115200);
    uart_interrupt_init();

    // initialize sensors
    oi_t *sensor_data = oi_alloc();
    oi_init(sensor_data);

    // initialize scan sensors
    cyBOT_init_Scan();
    lcd_clear();
    sendUartString("MAN,0\n");

    // calibration for servo
    left_calibration_value = 1272250;
    right_calibration_value = 301000;
    cyBOT_Scan_t scanData;

    char returnString[30];
    int bumperStat = 0;

    while(1)
    {
        oi_update(sensor_data);
        oi_setWheels(0, 0); // stop
        if (bumperStat != 1 && sensor_data->bumpLeft) {
            sendUartString("BMP,-1\n");
            bumperStat = 1;
        } else if (bumperStat != 2 && sensor_data->bumpRight) {
            sendUartString("BMP,1\n");
            bumperStat = 2;
        } else if (bumperStat > 0 && !(sensor_data->bumpRight || sensor_data->bumpLeft)){
            sendUartString("BMP,0\n");
            bumperStat = 0;
        }
        char rcdata = receive_data;
        receive_data = '\0';
        switch ( rcdata )
        {
        case 'w':; //move forward
            move(sensor_data, 10);
            break;
        case 's':; //move back
            move(sensor_data, -10);
            break;
        case 'd':; //turn right
            turn(sensor_data, -12);
            break;
        case 'a':; //turn left
            turn(sensor_data, 12);
            break;
        case 'm':; //scan
            int theta;
            for (theta = 0; theta <= 180; theta += 2 ) {
                cyBOT_Scan(theta, &scanData); // scan at theta degrees.
                timer_waitMillis(100); // wait so it doesn't break
                sprintf(returnString, "SCN,%d,%0.2f,%0.2f\n",theta,convertIRToDist(scanData.IR_raw_val),scanData.sound_dist);
                sendUartString(returnString);
            }
            break;
        case 't':;
            sendUartString("AUT,0\n");
            int startTheta[MAX_OBJECTS] = {0,0,0,0,0,0,0,0,0,0}; // store start theta
            int endTheta[MAX_OBJECTS] = {0,0,0,0,0,0,0,0,0,0}; // store end theta
            float hypotenuse[MAX_OBJECTS] = {0,0,0,0,0,0,0,0,0,0}; // store length of hypotenuse

            // scan 360 degrees around
            int objCounter = scanInfront(startTheta, endTheta, hypotenuse, 0, 0);
            if (receive_data == 't') {
                sendUartString("MAN,0\n");
                receive_data = '\0';
                break;
            }
            turn(sensor_data, 180);
            if (receive_data == 't') {
                sendUartString("MAN,0\n");
                receive_data = '\0';
                break;
            }
            int totalObj = scanInfront(startTheta, endTheta, hypotenuse, 182, objCounter);
            if (receive_data == 't') {
                sendUartString("MAN,0\n");
                receive_data = '\0';
                break;
            }

            float smallWidth = 0;
            int smallIndex = 0;

            int i;
            for (i = 0; i < totalObj; i++) {
               float base = calculateActualWidth(startTheta[i], endTheta[i], hypotenuse[i]);

               //find smallest object
               if ((smallWidth > base || smallWidth == 0) && base > 1) {
                   smallWidth = base;
                   smallIndex = i;
               }
            }
            sprintf(returnString,"%d,%d\n",startTheta[smallIndex],endTheta[smallIndex]);
            sendUartString(returnString);

            int turnVal = ((startTheta[smallIndex] + endTheta[smallIndex]) / 2);

            if ( turnVal > 270) {
                turn(sensor_data, turnVal - 270);
            } else {
                turn(sensor_data, (-1) * (270 - turnVal));
            }
            if (receive_data == 't') {
                sendUartString("MAN,0\n");
                receive_data = '\0';
                break;
            }

            cyBOT_Scan_t scanData;
            cyBOT_Scan(90, &scanData); // scan at theta degrees.
            float returnDist = scanData.sound_dist - 10;

            int offset = 0;
            float totalDist = 0;

            while (totalDist < returnDist*10) {
                oi_setWheels(50, 50);
                oi_update(sensor_data);
                if (sensor_data->bumpLeft) {
                    sendUartString("BMP,-1\n");
                    sprintf(returnString,"MOV,%0.2f\n",((float)sensor_data->distance / 10));
                    sendUartString(returnString);

                    move(sensor_data, -10);
                    sendUartString("BMP,0\n");

                    turn(sensor_data, -90);
                    move(sensor_data, 25);
                    turn(sensor_data, 90);

                    returnDist += 100;
                    offset += 25;
                }
                else if(sensor_data->bumpRight) {
                    sendUartString("BMP,1\n");
                    sprintf(returnString,"MOV,%0.2f\n",((float)sensor_data->distance / 10));
                    sendUartString(returnString);

                    move(sensor_data, -10);
                    sendUartString("BMP,0\n");

                    turn(sensor_data, 90);
                    move(sensor_data, 25);
                    turn(sensor_data, -90);

                    returnDist += 150;
                    offset -= 25;
                } else {
                    sprintf(returnString,"MOV,%0.2f\n",((float)sensor_data->distance / 10));
                    sendUartString(returnString);
                }
                totalDist += sensor_data->distance;
            }
            if (offset > 0) {
                turn(sensor_data,90);
                move(sensor_data,abs(offset));
            } else if (offset < 0) {
                turn(sensor_data,-90);
                move(sensor_data,abs(offset));
            }
            sendUartString("MAN,0\n");
            break;
        }
    }
}
