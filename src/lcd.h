/*
 * spi.h
 *
 *  Created on: Nov 13, 2018
 *      Author: jtst
 */

#ifndef LCD_H_
#define LCD_H_

#include <stdint.h>

void lcd_delay_ns(); //delay function using TIM3
void lcd_send_packet(uint8_t); //sends triple 8-bit SPI transmission
void lcd_send_command(uint8_t); //sends an LCD command (in 4-bit mode)
void lcd_set_address(uint8_t); //sets the current character address
void lcd_write_char(char); //writes a character to the screen
void lcd_write_line(char*, int); //writes length-8 strings to a display line
void lcd_write_resistance(unsigned int); //writes a resistance value to the screen
void lcd_write_frequency(unsigned int); //writes a frequency value to the screen
void lcd_config(); //configures the display

#endif /* LCD_H_ */
