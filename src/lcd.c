/*
 * spi.c
 *
 *  Created on: Nov 13, 2018
 *      Author: jtst
 */
#include <stdio.h>
#include <stdlib.h>
#include "diag/Trace.h"
#include "cmsis/cmsis_device.h"
#include "init.h"

void lcd_send_packet(uint8_t data)


{
	//uses full 8-bit interface !!!


	//we ignore the top bit of the packet
	data &= ~(0x80);

	//first send

	//LCK low
	GPIOB->BSRR |= GPIO_BSRR_BR_7;

	//wait for SPI1 ready
	while(SPI1->SR | SPI_SR_BSY);

	//send with MSB = 0
	uint8_t p = (0x80 | data);
	SPI_SendData8(SPI1, p);

	//wait for SPI1 ready
	while(SPI1->SR | SPI_SR_BSY);

	//LCK high
	GPIOB->BSRR |= GPIO_BSRR_BS_7;

	//delay by starting, then polling TIM3
	TIM3->CR1 |= TIM_CR1_CEN;
	while(0 != TIM3->CNT); //wait for count to hit zero


	//second send

	//LCK low
	GPIOB->BSRR |= GPIO_BSRR_BR_7;

	//wait for SPI1 ready
	while(SPI1->SR | SPI_SR_BSY);

	//send with MSB=0
	SPI_SendData8(SPI1, data);

	//wait for SPI1 ready
	while(SPI1->SR | SPI_SR_BSY);

	//LCK high
	GPIOB->BSRR |= GPIO_BSRR_BS_7;

	//delay by starting, then polling TIM3
	TIM3->CR1 |= TIM_CR1_CEN;
	while(0 != TIM3->CNT); //wait for count to hit zero


	//third send


	//LCK low
	GPIOB->BSRR |= GPIO_BSRR_BR_7;

	//wait for SPI1 ready
	while(SPI1->SR | SPI_SR_BSY);

	//send with MSB=1
	p = (0x80 | data);
	SPI_SendData8(SPI1, p);

	//wait for SPI1 ready
	while(SPI1->SR | SPI_SR_BSY);

	//LCK high
	GPIOB->BSRR |= GPIO_BSRR_BS_7;

	//done!!

}

void lcd_send_command(uint8_t cmd){
	//uses 4-bit interface, make sure to set that up before using
	/*
	 * 	address to display character mapping:
	 *
	 * | 00 | 01 | 02 | 03 | 04 | 05 | 06 | 07 |
	 * | 40 | 41 | 42 | 43 | 44 | 45 | 46 | 47 |
	 *
	 */

	uint8_t H = cmd >> 4;
	uint8_t L = cmd | 0xf;

	lcd_send_packet(H);
	lcd_send_packet(L);
}

void lcd_set_address(uint8_t addr){
	//top bit is ignored, set to 1 to make the address change command
	addr |= 0x80;
	lcd_send_command(addr);
}

void lcd_write_char(char ch){
	//address should auto increment after each character write

	uint8_t H = ((uint8_t) ch) >> 4;
	uint8_t L = ((uint8_t) ch) | 0xf;

	uint8_t packet = 0x40 | H;
	lcd_send_packet(packet);

	packet = 0x40 | L;
	lcd_send_packet(packet);
}

void lcd_config(){
	SPI_init();

	//switch to 4-bit LCD interface
	lcd_send_packet(0x20);

	lcd_send_command(0x28); //DL = 0 (4-bit interface), N=1 (two lines), F=0 (doesn't matter)
	lcd_send_command(0x0C); //D = 1 (display on), C = 0 (cursor off), B = 0 (no blink)
	lcd_send_command(0x06); //I/D = 1 (auto-increment), S = 0 (no shift)
	lcd_send_command(0x01); //clear display

	//move to address zero
	lcd_set_address(0);

	//write test character
	lcd_write_char('a');


}



