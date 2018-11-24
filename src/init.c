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
	trace_printf("intializing GPIOA\n");
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
	ADC1->CR |= (ADC_CR_ADEN); //calibrate and enable
	while( ( (ADC1->ISR | ADC_ISR_ADRDY) == 0) && ((ADC1->CR | ADC_CR_ADCAL) == 0) ); //wait for startup to finish
	trace_printf("\tADC enabled on pin A07\n");

	ADC1->CR |= ADC_CR_ADSTART; //start it up!!

	//DAC to optocoupler on PA4

	//turn on the clock to the DAC
	RCC->APB1ENR |= RCC_APB1ENR_DACEN;

	//analog mode and no pullup or pulldown
	GPIOA->MODER |= GPIO_MODER_MODER4;
	GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPDR4);

	//enable DAC (automatically on PA4)
	DAC->CR |= DAC_CR_EN1 | DAC_CR_BOFF1;
	trace_printf("\tDAC enabled on pin A04\n");


	//frequency measurement on PA1

	//input mode
	GPIOA->MODER &= ~(GPIO_MODER_MODER1_1);

	//no pull up or pull down
	GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPDR0);
	trace_printf("\tfreq. measurement on pin A01");


	trace_printf("GPIOA  configured \n");
}

void GPIOB_init(){
	//enable clock to GPIOB
	RCC->AHBENR |= RCC_AHBENR_GPIOBEN;

	//configure PB4 as LCK (using regular GPIO out with pull-down)
	GPIOB->MODER &= ~(GPIO_MODER_MODER4_1);
	GPIOB->MODER |= GPIO_MODER_MODER4_0;
	GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPDR4);

	//configure the SPI MOSI and SCK pins

	GPIO_InitTypeDef sck_mosi_init_struct;

	sck_mosi_init_struct.GPIO_Pin = SPI1_MOSI_PIN | SPI1_SCK_PIN;
	sck_mosi_init_struct.GPIO_Mode = GPIO_Mode_AF;
	sck_mosi_init_struct.GPIO_Speed = GPIO_Speed_50MHz;
	sck_mosi_init_struct.GPIO_OType = GPIO_OType_PP;
	sck_mosi_init_struct.GPIO_PuPd = GPIO_PuPd_NOPULL;

	GPIO_Init(GPIOB, &sck_mosi_init_struct);

	trace_printf("GPIOB configured \n");

}

void SPI_init(){
	//enable clock to SPI1
	RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;

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
	SPI_Cmd(SPI1, ENABLE);

	while(SPI1->SR & SPI_SR_BSY);

	trace_printf("SPI interface configured\n");

}

void TIM3_init(){
	//enable the clock to TIM3
	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;

	TIM3->CR1 = (uint16_t) 0x008C;//buffer auto-reload, count up, stop on OF, interrupt on OF only

	TIM3->PSC = (uint16_t) 47999/2; //prescale to 1us clock

	TIM3->ARR = (uint32_t) 100; //overflow at 1ms. could make this shorter if needed.

	TIM3->EGR |= 0x1;

	trace_printf("TIM3 configured\n");

}

void TIM2_init(){
	//enable the clock to TIM2
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

	TIM2->CR1 = ((uint16_t) 0x008C); //buff auto-reload, count up, stop on OF, interrupt on OF only

	//don't scale clock
	TIM2->PSC = (uint16_t) 0x0000;

	//set auto-reload (overflow) delay to max possible
	TIM2->ARR = (uint32_t) 0xFFFFFFFF;

	//force an update to the timer registers
	TIM2->EGR = (uint16_t) 0x0001; //triggers UG bit

	//assign the interrupt priority to 0 in the NVIC
	NVIC_SetPriority(TIM2_IRQn, 0);

	//enable TIM2 interrupts in the NVIC
	NVIC_EnableIRQ(TIM2_IRQn);

	//enable interrupt generation from TIM2
	TIM2->DIER |= TIM_DIER_UIE;

	//start the timer
	TIM2->CR1 |= TIM_CR1_CEN;

	trace_printf("TIM2 configured\n");
}

void EXTI_init(){
	//map EXTI1 line to PA01
	SYSCFG->EXTICR[0] = SYSCFG_EXTICR1_EXTI1_PA;

	//set EXTI1 line to interrupt on rising edge
	EXTI->RTSR = EXTI_RTSR_TR1;

	//unmask interrupts from EXTI1 line
	EXTI->IMR = EXTI_IMR_MR1;

	//assign the interrupt priority to 0 in the NVIC
	NVIC_SetPriority(EXTI0_1_IRQn, 0);

	//enable EXTI1 interrupts in the NVIC
	NVIC_EnableIRQ(EXTI0_1_IRQn);

	trace_printf("EXTI1 configured\n");
}


