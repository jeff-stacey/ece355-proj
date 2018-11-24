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



#define INTER_SEND_DELAY (5)
#define LCD_CLEAR_COMMAND (0x01)
#define MAX_DISPLAY_RESISTANCE (9999)
#define MAX_DISPLAY_FREQUENCY (99999)

void lcd_delay_ns(int n){
	//delay by starting, then polling TIM3
	TIM3->ARR = n;
	TIM3->CNT = 0;
	TIM3->EGR |= 1; //update values
	TIM3->CR1 |= TIM_CR1_CEN; //start
	while(!(TIM3->SR & TIM_SR_UIF)); //wait for interrupt flag to be set
	TIM3->SR &= ~(TIM_SR_UIF); //clear interrupt flag
	return;
}

void lcd_send_packet(uint8_t data)
{
	//trace_printf("sending packet: %x\n", data);
	//uses full 8-bit interface !!!

	//first send
	//trace_printf("\twrite: %x\n", data);

	//LCK low
	GPIOB->BRR |= SPI1_LCK_PIN;

	//wait for SPI1 ready
	//while(0 == (SPI1->SR & SPI_SR_TXE));
	while((SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET) &&
			SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET);

	//send with MSB=0
	SPI_SendData8(SPI1, data);

	//wait for SPI1 ready
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET);

	//LCK high
	GPIOB->BSRR |= SPI1_LCK_PIN;


	lcd_delay_ns(INTER_SEND_DELAY);


	//second send

	//send with MSB = 1
	uint8_t p = (0x80 | data);
	//trace_printf("\twrite: %x\n", p);

	//LCK low
	GPIOB->BRR |= SPI1_LCK_PIN;

	//wait for SPI1 ready
	while((SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET) &&
			SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET);

	SPI_SendData8(SPI1, p);

	//wait for SPI1 ready
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET);

	//LCK high
	GPIOB->BSRR |= SPI1_LCK_PIN;

	//wait
	lcd_delay_ns(INTER_SEND_DELAY);


	//third send
	//trace_printf("\twrite: %x\n", data);

	//LCK low
	GPIOB->BSRR |= GPIO_BSRR_BR_4;

	//wait for SPI1 ready
	while((SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET) &&
			SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET);

	//send with MSB=0
	SPI_SendData8(SPI1, data);

	//wait for SPI1 ready
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET);

	//LCK high
	GPIOB->BSRR |= SPI1_LCK_PIN;

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

	uint8_t H = (cmd & 0xf0) >> 4;
	uint8_t L = cmd & 0xf;

	//trace_printf("sending command: %x (%x %x)\n", cmd, H, L);

	lcd_send_packet(H);
	lcd_send_packet(L);
}

void lcd_set_address(uint8_t addr){
	//top bit is ignored, set to 1 to make the address change command
	addr |= 0x80;
	lcd_send_command(addr);
}

void lcd_write_char(uint8_t ch){
	//since chars and unit8_t are the same, this command works with numbers and single-quote char literals
	//address should auto increment after each character write

	uint8_t H = (((uint8_t) ch) & 0xf0) >> 4;
	uint8_t L = ((uint8_t) ch) & 0xf;

	//trace_printf("sending character %c (%x %x)\n", ch, H, L);

	uint8_t packet = 0x40 | H;
	lcd_send_packet(packet);

	packet = 0x40 | L;
	lcd_send_packet(packet);
}

void lcd_write_line(char* input_string, int line_no){
	//writes a string into a line of the display
	//line_no should be 0 or 1

	int j;

	int starting_address = line_no * 0x40;

	lcd_set_address(starting_address);

	char padded_string[8];
	sprintf(padded_string, "%8s", input_string);

	for(j = 0; j < 8; j++){

		lcd_write_char(padded_string[j]);
	}
}

void lcd_write_resistance(unsigned int r){
	char resistance_string[8];
	if(r > MAX_DISPLAY_RESISTANCE){
		sprintf(resistance_string, "* OL *");
	}
	else {
		sprintf(resistance_string, "%d Ohm", r);
	}
	lcd_write_line(resistance_string, 0);
}

void lcd_write_frequency(unsigned int f){
	char freq_string[8];
	if((f > MAX_DISPLAY_FREQUENCY) || f == 0){
		sprintf(freq_string, "* OL *");
	}
	else {
		sprintf(freq_string, "%d Hz ", f);
	}
	lcd_write_line(freq_string, 1);

}

void lcd_clear_screen(){
	lcd_send_command(LCD_CLEAR_COMMAND);
}

void lcd_config(){
	//switch to 4-bit LCD interface
	lcd_send_packet(0x2);

	lcd_send_command(0x28); //DL = 0 (4-bit interface), N=1 (two lines), F=0 (doesn't matter)
	lcd_send_command(0x0C); //D = 1 (display on), C = 0 (cursor off), B = 0 (no blink)
	lcd_send_command(0x06); //I/D = 1 (auto-increment), S = 0 (no shift)
	lcd_send_command(LCD_CLEAR_COMMAND); //clear display

	lcd_set_address(0x0);

}





