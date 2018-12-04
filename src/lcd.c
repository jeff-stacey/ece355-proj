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
	TIM3->ARR = n; //number of us to wait
	TIM3->CNT = 0; //clear count
	TIM3->EGR |= 1; //update values
	TIM3->CR1 |= TIM_CR1_CEN; //start
	while(!(TIM3->SR & TIM_SR_UIF)); //wait for interrupt flag to be set
	TIM3->SR &= ~(TIM_SR_UIF); //clear interrupt flag
	return;
}

void lcd_send_packet(uint8_t data)
{
	//first send
	GPIOB->BRR |= SPI1_LCK_PIN; //LCK low
	//wait for SPI1 ready
	while((SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET) &&
			SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET);
	SPI_SendData8(SPI1, data); //send with EN = 0
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET); //wait for SPI1 ready
	GPIOB->BSRR |= SPI1_LCK_PIN; //LCK high
	lcd_delay_ns(INTER_SEND_DELAY); //delay before next transmission
	
    //second send
	uint8_t p = (0x80 | data); //set EN
	GPIOB->BRR |= SPI1_LCK_PIN; //LCK low
	//wait for SPI1 ready
	while((SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET) &&
			SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET);
	SPI_SendData8(SPI1, p); //send with EN = 1
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET); //wait for SPI1 ready
	GPIOB->BSRR |= SPI1_LCK_PIN; //LCK high
	lcd_delay_ns(INTER_SEND_DELAY); //wait before next transmission
	
    //third send
	GPIOB->BSRR |= GPIO_BSRR_BR_4;//LCK low
	//wait for SPI1 ready
	while((SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET) &&
			SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET);
	SPI_SendData8(SPI1, data); //send with MSB=0
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET); //wait for SPI1 ready
	GPIOB->BSRR |= SPI1_LCK_PIN; //LCK high
    //packet sent!
}

void lcd_send_command(uint8_t cmd){
	//uses 4-bit interface, make sure to set that up before using
	uint8_t H = (cmd & 0xf0) >> 4; //split command to send on 4-bit interface
	uint8_t L = cmd & 0xf;
	lcd_send_packet(H); //send high half
	lcd_send_packet(L); //send low half
}

void lcd_set_address(uint8_t addr){
	 /* 	address to display character mapping:
	 *  | 00 | 01 | 02 | 03 | 04 | 05 | 06 | 07 |
	 *  | 40 | 41 | 42 | 43 | 44 | 45 | 46 | 47 |
	 */
	addr |= 0x80; //set top bit to 1 to make the address change command
	lcd_send_command(addr); //send the command
}

void lcd_write_char(uint8_t ch){
	//since chars and unit8_t are the same, this command works with numbers and single-quote char literals
	//address should auto increment after each character write
	
    uint8_t H = (((uint8_t) ch) & 0xf0) >> 4; //split write command into two parts before sending
	uint8_t L = ((uint8_t) ch) & 0xf; 
	uint8_t packet = 0x40 | H; //build high half character write command
	lcd_send_packet(packet); //send high half
	packet = 0x40 | L; //build low half character write command
	lcd_send_packet(packet); //send low half
}

void lcd_write_line(char* input_string, int line_no){
	//writes a string into a line of the display
	//line_no: 0 is top line, 1 is bottom
	int j; //index variable
	int starting_address = line_no * 0x40; //start at the beginning of the line
	lcd_set_address(starting_address); //start writing on the right line
	char padded_string[8]; //temporary string storage
	sprintf(padded_string, "%8s", input_string); //we must print exactly 8 characters to the line
	for(j = 0; j < 8; j++){ lcd_write_char(padded_string[j]); } //write each character of the string
}

void lcd_write_resistance(unsigned int r){
	char resistance_string[8]; //this is the string we'll pass to lcd_write_line
	if(r > MAX_DISPLAY_RESISTANCE){ //if resistance is too big,
		sprintf(resistance_string, "** OL **"); //print overload message
	}
	else { //otherwise 
		sprintf(resistance_string, "%4d Ohm", r); //print the number then the unit
	}
	lcd_write_line(resistance_string, 0); //send to the first line
}

void lcd_write_frequency(unsigned int f){
	char freq_string[8]; //this is the string we'll pass to lcd_write_line
	if((f > MAX_DISPLAY_FREQUENCY) || f == 0){ //if the frequency is out of range or the timer overflowed
		sprintf(freq_string, "** OL  *"); //print the overload message
	}
	else { //otherwise
		sprintf(freq_string, "%4d Hz ", f); //print the number then the unit
	}
	lcd_write_line(freq_string, 1); //send to the second line
}

void lcd_config(){
	lcd_send_packet(0x2); //switch to 4-bit LCD interface
	lcd_send_command(0x28); //DL = 0 (4-bit interface), N=1 (two lines), F=0 (doesn't matter)
	lcd_send_command(0x0C); //D = 1 (display on), C = 0 (cursor off), B = 0 (no blink)
	lcd_send_command(0x06); //I/D = 1 (auto-increment), S = 0 (no shift)
	lcd_send_command(LCD_CLEAR_COMMAND); //clear display
	lcd_set_address(0x0); //start writing
}





