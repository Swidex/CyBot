#include <stdio.h>
#include "open_interface.h"
#include "Timer.h"
#include "lcd.h"

double move(oi_t *sensor, int centimeters);
    // moves the bot [centimeters]
    // centimeters > 0 = forwards, centimeters < 0 = backwards

double turn(oi_t *sensor, double degrees);
    // turns the bot [degrees]

void moveAndAvoid(oi_t *sensor, double returnDist);
