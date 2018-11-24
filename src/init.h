/*
 * init.h
 *
 *  Created on: Oct 24, 2018
 *      Author: jtst
 */

#ifndef INIT_H_
#define INIT_H_

//GPIO initialization

#define SPI1_MOSI_PIN (GPIO_Pin_5)
#define SPI1_SCK_PIN (GPIO_Pin_3)
#define SPI1_LCK_PIN (GPIO_Pin_4)

#define TIM2_OFLOW_VALUE ((uint32_t) 0xFFFFFFFF)

void GPIOA_init();

void GPIOB_init();

void SPI_init();

void TIM3_init();

void TIM2_init();

void EXTI_init();



#endif /* INIT_H_ */
