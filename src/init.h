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

//Initializes GPIOA pins and associated peripherals
// --> A07 and ADC ch. 7, A04 and DAC, A01
void GPIOA_init(); 

//Initializes GPIOB pins (for SPI/LCD communication)
//B05 is MOSI, B03 is SCK, and B04 is LCK
void GPIOB_init();

//Initializes SPI peripheral for LCD communication
void SPI_init();

//Initializes TIM3 for use as a delay
//Prescaled to 1us/tick with 100us overflow limit
void TIM3_init();

//Initializes TIM2 for frequency measurement
//No prescaling, interrupt on overflow
void TIM2_init();

//Initialize EXTI for frequency measurement
//interrupt on rising edge of pin A01
void EXTI_init();

#endif /* INIT_H_ */
