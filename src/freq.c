/*
 * freq.c
 *
 *  Created on: Nov 23, 2018
 *      Author: jtst
 */


#include <stdio.h>
#include <stdlib.h>
#include "diag/Trace.h"
#include "cmsis/cmsis_device.h"
#include "stm32f0-stdperiph/stm32f0xx_spi.h"

extern unsigned int period_ticks;
int first_edge = 1;

void TIM2_IRQHandler(){
	//this handler is run if the timer overflows
	//that means that the frequency is too low for us to measure

	//check that the interrupt flag is set
	if((TIM2->SR & TIM_SR_UIF) != 0){
		period_ticks = 0;
		//we didn't record any ticks, so write zero to the global var

		//clear the interrupt flag
		TIM2->SR &= ~(TIM_SR_UIF);

		//restart the timer
		TIM2->CR1 |= TIM_CR1_CEN;
	}
}

void EXTI0_1_IRQHandler(){
	//runs when an edge is detected

	unsigned int local_period_ticks;

	if((EXTI->PR & EXTI_PR_PR1) != 0){

		local_period_ticks = TIM2->CNT; //get ticks since last edge
		TIM2->CNT = 0; //restart counting

		//if this is the second measured edge, we have the period in the counter register
		if(0 == first_edge){
			//assuming 50% duty cycle
			period_ticks = local_period_ticks; //pass the data to be displayed
		}

		first_edge = !first_edge; //if we started counting this time, we should stop next time

		//clear the interrupt flag
		EXTI->PR |= EXTI_PR_PR1;
	}
}
