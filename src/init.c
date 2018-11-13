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

	//analog mode and no pullup or pulldown
	GPIOA->MODER |= GPIO_MODER_MODER7;
	GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPDR7);

	//configure ADC
	ADC1->CFGR1 |= ADC_CFGR1_CONT; //continuous conversion
	ADC1->CHSELR |= ADC_CHSELR_CHSEL7;
	trace_printf("enabling ADC on pin A07\n");
	ADC1->CR |= (ADC_CR_ADEN); //calibrate and enable
	while( ( (ADC1->ISR | ADC_ISR_ADRDY) == 0) && ((ADC1->CR | ADC_CR_ADCAL) == 0) ); //wait for startup to finish
	trace_printf("ADC enabled on pin A07\n");

	ADC1->CR |= ADC_CR_ADSTART; //start it up!!

	//DAC to optocoupler on PA4

	//turn on the clock to the DAC
	RCC->APB1ENR |= RCC_APB1ENR_DACEN;

	trace_printf("enabling DAC on pin A04\n");
	//analog mode and no pullup or pulldown
	GPIOA->MODER |= GPIO_MODER_MODER4;
	GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPDR4);

	//enable DAC (automatically on PA4)
	DAC->CR |= DAC_CR_EN1 | DAC_CR_BOFF1;


}

