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

/** Simple C8051F300 sdcc uart example
 * This example shows how to use the UART to transmit data from the mcu.
 * The code will send 0 and 1 with a delay in a loop.
 *
 * Compile with "sdcc uart.c" and flash uart.ihx
 * 
 */

#include <C8051F300.h>

void delay(int x)
{
    while(x--);
}


void main (void)
{
	PCA0MD &= ~0x40;			// Disable the watchdog timer

	/* Clock init */
	OSCICN = 0x17;				// Enable internal oscillator (24.5 MHz)
								// Set sysclock to internal oscillator
	RSTSRC = 0x04;				// Enable missing clock detector

	/* Port/Crossbar init */
	P0MDOUT = 0x10;				// Set port P0.4 (Uart tx) to push-pull
	XBR0 = 0xCF;				// UART is always on P0.4 and P0.5
	XBR1 = 0x03;				// Enable UART RX and TX
	XBR2 = 0x40;				// Enable the crossbar

	/* Configure UART for a baud rate of 115200 */
	SCON0 = 0x10;				// Enable UART reception
	CKCON = 0x10;				// Timer 1 uses the sysclock
	TH1 = 0x96;					// 115200 baud rate reload value
	TMOD = 0x20;				// Select 8-bit counter for Timer 1
	TR1 = 1;					// Enable Timer 1

	while (1) {					// Loop forever
		SBUF0 = '0';			// Set all gpio pins to 0
		delay(10000);			// Delay for some time

		SBUF0 = '1';			// Set all gpio pins to 0
		delay(10000);			// Delay for some time
   }
}
