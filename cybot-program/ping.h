/*
 * ping.h
 *
 *  Created on: Oct 27, 2021
 *      Author: benhall
 */

#ifndef PING_H_
#define PING_H_

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <inc/tm4c123gh6pm.h>
#include "timer.h"

void ping_init();

float ping_read();

void send_pulse();

void TIMER3B_Handler();

#endif /* PING_H_ */
