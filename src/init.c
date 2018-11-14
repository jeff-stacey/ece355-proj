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

void SPI_init(){
	//enable clock to GPIOB and SPI1
	RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
	RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;

	//set PB3 and PB5 to be SCK and MOSI
	GPIOB->MODER |= GPIO_MODER_MODER3_0;
	GPIOB->MODER &= ~(GPIO_MODER_MODER3_1);

	GPIOB->MODER |= GPIO_MODER_MODER5_0;
	GPIOB->MODER &= ~(GPIO_MODER_MODER5_1);

	SPI_InitTypeDef init_struct;

	init_struct.SPI_Direction = SPI_Direction_1Line_Tx;
	init_struct.SPI_Mode = SPI_Mode_Master;
	init_struct.SPI_DataSize = SPI_DataSize_8b;
	init_struct.SPI_CPOL = SPI_CPOL_Low;
	init_struct.SPI_CPHA = SPI_CPHA_1Edge;
	init_struct.SPI_NSS = SPI_NSS_Soft;
	init_struct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
	init_struct.SPI_FirstBit = SPI_FirstBit_MSB;
	init_struct.SPI_CRCPolynomial = 7;

	SPI_Init(SPI1, &init_struct);
	SPI_Cmd(SPI1, ENABLE);

	//configure PB7 as LCK (using regular GPIO out)
	GPIOB->MODER &= ~(GPIO_MODER_MODER7_1);
	GPIOB->MODER |= GPIO_MODER_MODER7_0;

	GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPDR7_0);
	GPIOB->PUPDR |= GPIO_PUPDR_PUPDR7_1;

	while(SPI1->SR | SPI_SR_TXE);
	while(SPI1->SR | SPI_SR_BSY);

	trace_printf("SPI interface configured\n");

}

void TIM3_init(){
	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;

	TIM3->CR1 = (uint16_t) 0x009C; //buffer auto-reload, count down, stop on OF, interrupt on OF only

	TIM3->PSC = 0x00; //no prescaling

	TIM3->ARR = 48000; //overflow at 1ms. could make this shorter if needed.

	TIM3->EGR = 0; //no interrupts from TIM3


}


