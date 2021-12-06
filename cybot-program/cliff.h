#ifndef CLIFF_H_
#define CLIFF_H_

#include <stdint.h>
#include <inc/tm4c123gh6pm.h>
#include <stdbool.h>

unsigned int updateCliffStatus(oi_t *sensor);

void cliff_set_calibration(int min, int max);

#endif
