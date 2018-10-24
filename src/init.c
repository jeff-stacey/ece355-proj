/*
 * init.c
 *
 *  Created on: Oct 24, 2018
 *      Author: jtst
 */

#include <stdio.h>
#include <stdlib.h>
#include "diag/Trace.h"
#include "cmsis/cmsis_device.h"

void GPIOA_Init(){
	//turn on the clock to GPIOA
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
	RCC->APB2ENR |= RCC_APB2ENR_ADCEN;

	//potentiometer and ADC on PA07

	//input mode and no pullup or pulldown
	GPIOA->MODER |= GPIO_MODER_MODER7;
	GPIOA->MODER &= ~(GPIO_PUPDR_PUPDR7);

	//configure ADC
	ADC1->CFGR1 |= ADC_CFGR1_CONT; //continuous conversion
	ADC1->CHSELR |= ADC_CHSELR_CHSEL7;
	trace_printf("enabling ADC on pin A07\n");
	ADC1->CR |= (ADC_CR_ADEN); //calibrate and enable
	while( ( (ADC1->ISR | ADC_ISR_ADRDY) == 0) && ((ADC1->CR | ADC_CR_ADCAL) == 0) ); //wait for startup to finish
	trace_printf("ADC enabled on pin A07\n");

	ADC1->CR |= ADC_CR_ADSTART; //start it up!!
}

