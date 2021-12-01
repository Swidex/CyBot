#include "movement.h"

#include <stdio.h>
#include "open_interface.h"

#define CLIFF_MAX 2500
#define CLIFF_MIN 750

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
        if (cliffData[i] > CLIFF_MAX)
        {
            cliffReturn |= ( 0b1 << (i * 2) );
        } else if (cliffData[i] < CLIFF_MIN)
        {
            cliffReturn |= ( 0b1 << ((i * 2) + 1));
        }
    }

    return cliffReturn;
}
