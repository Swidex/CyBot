#include "movement.h"

#include <stdio.h>
#include "open_interface.h"



int cliff_min = 500;
int cliff_max = 2500;

unsigned int updateCliffStatus(oi_t *sensor) {
    unsigned int cliffReturn = 0b00000000;
    unsigned int cliffData[4] = {sensor->cliffLeftSignal,
                                 sensor->cliffFrontLeftSignal,
                                 sensor->cliffRightSignal,
                                 sensor->cliffFrontRightSignal
                                };
    int i;
    for (i = 0; i < 4; i++)
    {
        if (cliffData[i] > cliff_max)
        {
            cliffReturn |= ( 0b1 << (i * 2) );
        } else if (cliffData[i] < cliff_min)
        {
            cliffReturn |= ( 0b1 << ((i * 2) + 1));
        }
    }

    return cliffReturn;
}

void cliff_set_calibration(int min, int max) {
    cliff_min = min;
    cliff_max = max;

}
