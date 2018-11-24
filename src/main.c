/*
 * This file is part of the µOS++ distribution.
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

// ----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include "diag/Trace.h"
#include "cmsis/cmsis_device.h"

#include "init.h"
#include "lcd.h"
#include "freq.h"

// ----------------------------------------------------------------------------
//
// STM32F0 empty sample (trace via DEBUG).
//
// Trace support is enabled by adding the TRACE macro definition.
// By default the trace messages are forwarded to the DEBUG output,
// but can be rerouted to any device or completely suppressed, by
// changing the definitions required in system/src/diag/trace_impl.c
// (currently OS_USE_TRACE_ITM, OS_USE_TRACE_SEMIHOSTING_DEBUG/_STDOUT).
//

// ----- main() ---------------------------------------------------------------

// Sample pragmas to cope with warnings. Please note the related line at
// the end of this function, used to pop the compiler diagnostics status.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"

//#define DAC_WRITE_REG DAC->DHR12R1

#define POT_MAX_RESISTANCE (5000) //board pot is rated 5k
#define ADC_MAX_VALUE (0xFFF) //we're using 12-bit mode
#define RESISTANCE_MULTIPLIER ((float) POT_MAX_RESISTANCE / ADC_MAX_VALUE)

#define PERIOD_ERROR (8) //measured difference due to delay between counter read and counter start
	//TODO: measure the period error again
#define TICK_LENGTH_NS ((float) 20.833333333)
#define NS_TO_S_MULTIPLIER (1000000000)

unsigned int period_ticks;

int
main(int argc, char* argv[])
{
  // At this stage the system clock should have already been configured
  // at high speed.

	uint16_t dac_output_data;
	unsigned int measured_resistance;
	int local_period_ticks;
	float period_ns;
	unsigned int measured_freq;

	GPIOA_init();
	GPIOB_init();
	TIM3_init();
	SPI_init();
	lcd_config();

	TIM2_init();
	EXTI_init();


  // Infinite loop
	while (1)
	{
		//frequency measurement
		local_period_ticks = period_ticks; //grab this now so it's consistent even if an interrupt changes its value
		if(local_period_ticks == 0){
			measured_freq = 0;
		}
		else
		{
			period_ns = TICK_LENGTH_NS * (period_ticks + PERIOD_ERROR);
			measured_freq = (unsigned int) NS_TO_S_MULTIPLIER / (period_ns);
		}
		lcd_write_frequency(measured_freq);

		//resistance measurement
		dac_output_data = ADC1->DR;
		DAC->DHR12R1 = dac_output_data;
		measured_resistance = (unsigned int) (dac_output_data) * RESISTANCE_MULTIPLIER;
		lcd_write_resistance(measured_resistance);

	}
}





#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
