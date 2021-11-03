/*
 * servo.h
 *
 *  Created on: Nov 3, 2021
 *      Author: benhall
 */

#ifndef SERVO_H_
#define SERVO_H_

#include <inc/tm4c123gh6pm.h>
#include <stdbool.h>
#include <stdint.h>
#include <timer.h>
#include "driverlib/interrupt.h"

volatile float servo_pos;

void servo_init(void);

void servo_move(float degrees);

#endif /* SERVO_H_ */
