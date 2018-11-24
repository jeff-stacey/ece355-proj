/*
 * spi.h
 *
 *  Created on: Nov 13, 2018
 *      Author: jtst
 */

#ifndef LCD_H_
#define LCD_H_

#include <stdint.h>

void lcd_delay_ns();

void lcd_send_packet(uint8_t);

void lcd_send_command(uint8_t);

void lcd_set_address(uint8_t);

void lcd_write_char(char);

void lcd_write_line(char*);

void lcd_write_resistance(unsigned int);

void lcd_write_frequency(unsigned int);

void lcd_clear_screen();

void lcd_config();



#endif /* LCD_H_ */
