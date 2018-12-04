/* This file is part of the µOS++ distribution.
 *   (https://github.com/micro-os-plus)
 * Copyright (c) 2014 Liviu Ionescu.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include "diag/Trace.h"
#include "cmsis/cmsis_device.h"

#include "init.h"
#include "lcd.h"
#include "freq.h"

// Sample pragmas to cope with warnings. Please note the related line at
// the end of this function, used to pop the compiler diagnostics status.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"

#define POT_MAX_RESISTANCE (5000) //board pot is rated 5k
#define ADC_MAX_VALUE (0xFFF) //we're using 12-bit mode
#define RESISTANCE_MULTIPLIER ((float) POT_MAX_RESISTANCE / ADC_MAX_VALUE) 
//conversion factor from ADC measurement to resistance

#define PERIOD_ERROR (8) //measured difference due to delay between counter read and counter start
#define TICK_LENGTH_NS ((float) 20.833333333) //lengh of one TIM2 tick in ns
#define NS_TO_S_MULTIPLIER (1000000000) //conversion factor from nanoseconds to seconds

unsigned int period_ticks;

int
main(int argc, char* argv[])
{
  // At this stage the system clock should have already been configured
  // at high speed.

	uint16_t dac_output_data; //temporary storage for the measured voltage
	unsigned int measured_resistance; //calculated porentiometer resistance
	int local_period_ticks; //sample the global variable into this
	float period_ns; //calculated period in nanoseconds
	unsigned int measured_freq; //calculated frequency
	
    GPIOA_init(); //initalize GPIOA, ADC, DAC
	GPIOB_init(); //initialize GPIOB
	TIM3_init(); //initialize delay timer
	SPI_init(); //initialize SPI subsystem
	lcd_config(); //configure LCD
	TIM2_init(); //initialize freq. measurement timer
	EXTI_init(); //initialize freq. measurement interrupt line

	while (1) //main loop
	{
        //frequency measurement
		local_period_ticks = period_ticks; //grab the tick count in case it changes 
		if(local_period_ticks == 0){ //if the frequency was too low to measure
			measured_freq = 0; //print the overload message
		} else { //otherwise
			period_ns = TICK_LENGTH_NS * (period_ticks + PERIOD_ERROR); //convert the period to NS
			measured_freq = (unsigned int) NS_TO_S_MULTIPLIER / (period_ns); //calculate frequency
		}
		lcd_write_frequency(measured_freq); //print the frequency

		//resistance measurement
		dac_output_data = ADC1->DR; //read raw resistance data
		DAC->DHR12R1 = dac_output_data; //set output voltage
		measured_resistance = (unsigned int) (dac_output_data) * RESISTANCE_MULTIPLIER; //calculate resistance
		lcd_write_resistance(measured_resistance); //print the resistance
	}
}
#pragma GCC diagnostic pop
