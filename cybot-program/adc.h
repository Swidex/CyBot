#ifndef CYBOT_ADC_H_
#define CYBOT_ADC_H_

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <inc/tm4c123gh6pm.h>

// Initialize the ADC
void adc_init(void);

int adc_read(void);

#endif /* CYBOT_UART_H_ */
