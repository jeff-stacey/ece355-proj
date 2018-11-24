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

void GPIOA_init(){
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
	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;

	TIM3->CR1 = (uint16_t) 0x008C;//buffer auto-reload, count down, stop on OF, interrupt on OF only

	TIM3->PSC = 47999/2; //prescale to 1us clock

	TIM3->ARR = 100; //overflow at 1ms. could make this shorter if needed.

	TIM3->EGR |= 0x1;

	trace_printf("TIM3 configured \n");

}


