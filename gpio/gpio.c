/*
Copyright (C) 2016 Benjamin Larsson <benjamin@southpole.se>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software
and associated documentation files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom
the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or
substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/** Simple C8051F300 sdcc gpio example
 *
 * Read the gpio pins that have buttons connected.
 * If they are pressed light up the leds connected
 * to the gpio pins.
 *
 * Compile with "sdcc gpio.c" and flash gpio.ihx
 * 
 */

#include <C8051F300.h>

void main (void)
{
	PCA0MD &= ~0x40;			// Disable the watchdog timer

	/* Clock init */
	OSCICN = 0x04;				// Enable internal oscillator (24.5 MHz)
								// Set sysclock to internal oscillator / 8
	RSTSRC = 0x04;				// Enable missing clock detector

	/* Port/Crossbar init */
	XBR0 = 0xFF;				// Skip all pins in crossbar selection
	XBR2 = 0x40;				// Enable the crossbar

	while (1) {					// Loop forever
		if (P0_5)				// If button 1 is pressed
			P0_6 = 1;			// Light up green led
		else
			P0_6 = 0;			// Turn off green led

		if (P0_4)				// If button 2 is pressed
			P0_7 = 1;			// Light up yellow led
		else
			P0_7 = 0;			// Turn off green led
   }
}
