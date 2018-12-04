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
	if((TIM2->SR & TIM_SR_UIF) != 0){ //check that the interrupt flag is set
		period_ticks = 0; //we didn't record any ticks, so write zero to the global var
        first_edge = 1; //try again on the next cycle
		TIM2->SR &= ~(TIM_SR_UIF); //clear the interrupt flag
		TIM2->CR1 |= TIM_CR1_CEN;//restart the timer
	}
}

void EXTI0_1_IRQHandler(){
	//runs when an edge is detected
	unsigned int local_period_ticks;
	if((EXTI->PR & EXTI_PR_PR1) != 0){
		local_period_ticks = TIM2->CNT; //get ticks since last edge
		TIM2->CNT = 0; //restart counting
		if(0 == first_edge)	period_ticks = local_period_ticks; //if this is the second edge, store the tick count
		first_edge = !first_edge; //if we started counting this time, we should stop next time
		EXTI->PR |= EXTI_PR_PR1; //clear the interrupt flag
	}
}
