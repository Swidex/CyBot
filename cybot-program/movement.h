#include <stdio.h>
#include "open_interface.h"
#include "Timer.h"
#include "lcd.h"

void move(oi_t *sensor, int centimeters);
    // moves the bot [centimeters]
    // centimeters > 0 = forwards, centimeters < 0 = backwards

void turnLeft(oi_t *sensor, double degrees);
    // turns the bot left [centimeters]

void turnRight(oi_t *sensor, double degrees);
    // turns the bot right [centimeters]

void moveAndAvoid(oi_t *sensor, int centimeters);
