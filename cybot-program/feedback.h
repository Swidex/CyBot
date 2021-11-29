/*
 * feedback.h
 *
 *  Created on: Nov 29, 2021
 *      Author: teohys
 */

#ifndef FEEDBACK_H_
#define FEEDBACK_H_


#include <stdint.h>
#include <inc/tm4c123gh6pm.h>

void button_init();

uint8_t button_getButton();

char feedback();


#endif /* FEEDBACK_H_ */
