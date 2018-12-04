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
#include "stm32f0-stdperiph/stm32f0xx_spi.h"
#include "init.h"
#include "freq.h"

void GPIOA_init(){
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN; //turn on the clock to GPIOA

	//potentiometer and ADC on PA07
	RCC->APB2ENR |= RCC_APB2ENR_ADCEN; //turn on the clock to the ADC
	GPIOA->MODER |= GPIO_MODER_MODER7; //analog mode
	GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPDR7); //no pull

	//configure ADC
	ADC1->CFGR1 |= ADC_CFGR1_CONT; //continuous conversion
	ADC1->CHSELR |= ADC_CHSELR_CHSEL7; //channel 7
	ADC1->CR |= (ADC_CR_ADEN); //calibrate and enable
	while( ( (ADC1->ISR | ADC_ISR_ADRDY) == 0) && ((ADC1->CR | ADC_CR_ADCAL) == 0) ); //wait for startup to finish

	ADC1->CR |= ADC_CR_ADSTART; //start it up!!

	//DAC to optocoupler on PA4
	RCC->APB1ENR |= RCC_APB1ENR_DACEN; //turn on clock to DAC
	GPIOA->MODER |= GPIO_MODER_MODER4; //analog mode
	GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPDR4); //no pull
	DAC->CR |= DAC_CR_EN1 | DAC_CR_BOFF1; //enable DAC (automatically on PA4)
	
    //frequency measurement on PA1
	GPIOA->MODER &= ~(GPIO_MODER_MODER1_1); //input mode
	GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPDR0); //no pull up or pull down

	trace_printf("GPIOA  configured \n");
}

void GPIOB_init(){
	RCC->AHBENR |= RCC_AHBENR_GPIOBEN;//enable clock to GPIOB

	//configure PB4 as LCK (using regular GPIO out with no pull)
	GPIOB->MODER &= ~(GPIO_MODER_MODER4_1);
	GPIOB->MODER |= GPIO_MODER_MODER4_0;
	GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPDR4);

	//configure PB5 as MOSI (alternate function mode and no pull)
    GPIOB->MODER &= ~(GPIO_MODER_MODER5_0);
	GPIOB->MODER |= GPIO_MODER_MODER5_1;
    GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPDR5);

    //configure PB3 as SCK (alternate function and no pull)
    GPIOB->MODER &= ~(GPIO_MODER_MODER3_0);
	GPIOB->MODER |= GPIO_MODER_MODER3_1;
    GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPDR3);
	
    trace_printf("GPIOB configured \n");
}

void SPI_init(){
	RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;RCC_APB2ENR_SPI1EN;//enable clock to SPI1

    //CMSIS initialization of the SPI device
	SPI_InitTypeDef spi1_init_struct;
	spi1_init_struct.SPI_Direction = SPI_Direction_1Line_Tx;
	spi1_init_struct.SPI_Mode = SPI_Mode_Master;
	spi1_init_struct.SPI_DataSize = SPI_DataSize_8b;
	spi1_init_struct.SPI_CPOL = SPI_CPOL_Low;
	spi1_init_struct.SPI_CPHA = SPI_CPHA_1Edge;
	spi1_init_struct.SPI_NSS = SPI_NSS_Soft;
	spi1_init_struct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
	spi1_init_struct.SPI_FirstBit = SPI_FirstBit_MSB;
	spi1_init_struct.SPI_CRCPolynomial = 7;
	SPI_Init(SPI1, &spi1_init_struct);
	SPI_Cmd(SPI1, ENABLE); //SPI is now enabled
	while(SPI1->SR & SPI_SR_BSY); //wait for device to be ready
	trace_printf("SPI interface configured\n");

}

void TIM3_init(){
	//enable the clock to TIM3
	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;

	TIM3->CR1 = (uint16_t) 0x008C;//buffer auto-reload, count up, stop on OF, interrupt on OF only

	TIM3->PSC = (uint16_t) 24000; //prescale to 1us clock

	TIM3->ARR = (uint32_t) 100; //overflow at 1ms. could make this shorter if needed.

	TIM3->EGR |= 0x1;

	trace_printf("TIM3 configured\n");

}

void TIM2_init(){
	//enable the clock to TIM2
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

	TIM2->CR1 = ((uint16_t) 0x008C); //buff auto-reload, count up, stop on OF, interrupt on OF only
	TIM2->PSC = (uint16_t) 0x0000; //don't scale clock
	TIM2->ARR = (uint32_t) 0xFFFFFFFF; //set auto-reload (overflow) delay to max possible
	TIM2->EGR = (uint16_t) 0x0001; //force registers to update
	NVIC_SetPriority(TIM2_IRQn, 0); //assign the interrupt priority to 0 in the NVIC
	NVIC_EnableIRQ(TIM2_IRQn);	//enable TIM2 interrupts in the NVIC
	TIM2->DIER |= TIM_DIER_UIE;	//enable interrupt generation from TIM2
	TIM2->CR1 |= TIM_CR1_CEN;//start the timer

	trace_printf("TIM2 configured\n");
}

void EXTI_init(){
	SYSCFG->EXTICR[0] = SYSCFG_EXTICR1_EXTI1_PA; //map EXTI1 line to PA01
	EXTI->RTSR = EXTI_RTSR_TR1;	//set EXTI1 line to interrupt on rising edge
	EXTI->IMR = EXTI_IMR_MR1;	//unmask interrupts from EXTI1 line
	NVIC_SetPriority(EXTI0_1_IRQn, 0);	//assign the interrupt priority to 0 in the NVIC
	NVIC_EnableIRQ(EXTI0_1_IRQn);//enable EXTI1 interrupts in the NVIC

	trace_printf("EXTI1 configured\n");
}


