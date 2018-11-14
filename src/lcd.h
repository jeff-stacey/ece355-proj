/*
 * spi.h
 *
 *  Created on: Nov 13, 2018
 *      Author: jtst
 */

#ifndef LCD_H_
#define LCD_H_

#include <stdint.h>

void lcd_config();

void lcd_set_address(uint8_t addr);

void lcd_send_command(uint8_t cmd);

void lcd_write_char(char ch);

void lcd_send_packet(uint8_t data);

#endif /* LCD_H_ */
