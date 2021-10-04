#include <stdio.h>
#include "movement.h"
#include "open_interface.h"
#include "Timer.h"
#include "lcd.h"

// moves the bot [centimeters]
void move(oi_t *sensor, int centimeters){
    // postive cm   = forwards
    // negative cm  = backwards

    double sum = 0;
    if (centimeters > 0) {
        oi_setWheels(100, 100);
    } else {
        oi_setWheels(-100, -100);
    }
    while (sum < abs(centimeters*10) ) {
        oi_update(sensor);
        sum += abs(sensor->distance);
    }
    oi_setWheels(0, 0); // stop
}

// turns the bot [degrees]
void turn(oi_t *sensor, double degrees){
    // negative deg = right / clockwise
    // positive deg = left / counter-clockwise

    double currAngle = 0;
    if (degrees < 0) {
        oi_setWheels(-50, 50); // turn clockwise
        while (currAngle > degrees ) {
            oi_update(sensor);
            currAngle += sensor->angle;
        }
    } else {
        oi_setWheels(50, -50); // turn counter-clockwise
        while (currAngle < degrees ) {
            oi_update(sensor);
            currAngle += sensor->angle;
        }
    }
    oi_setWheels(0, 0); // stop
}

void moveAndAvoid(oi_t *sensor, int centimeters){
    double totalDist = 0;
    while (totalDist < abs(centimeters*10)) {
        oi_setWheels(500, 500);
        oi_update(sensor);
        if (sensor->bumpLeft) {
            move(sensor, -15);
            turnRight(sensor, 90);
            move(sensor, 25);
            turnLeft(sensor, -90);
            centimeters += 15;
        }
        if(sensor->bumpRight) {
            move(sensor, -15);
            turnLeft(sensor, -90);
            move(sensor, 25);
            turnRight(sensor, 90);
            centimeters += 15;
        }
        totalDist += abs(sensor->distance);
    }
    oi_setWheels(0, 0); // stop
}
