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

/** Simple C8051F300 sdcc interrupt example
 *
 * Configure 2 pins as edge-triggered interrupt sources (buttons).
 * Install 2 interrupt service routines, that toggles one led each.
 *
 * Compile with "sdcc interrupt.c" and flash interrupt.ihx
 * 
 */

#include <C8051F300.h>

/* Define 2 leds on P0 bits */
__sbit __at (0x86) LEDY;		// Yellow Led at P0.6
__sbit __at (0x87) LEDG;		// Green Led at P0.7

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

	/* Interrupts */
	TCON = 0x05;				// Select edge triggering for INT0 and INT1
	IT01CF = 0x45;				// Select INT0 on P0.4 (left button)
								// Select INT1 on P0.5 (right button)
	EX0 = 1;					// Enable INT0 interrupt
	EX1 = 1;					// Enable INT1 interrupt
	EA = 1;						// Enable global interrupts

	while (1);
}

/* Interrupt service routines */

void INT0_ISR (void) __interrupt 0
{
   LEDY = !LEDY;
}

void INT1_ISR (void) __interrupt 2
{
   LEDG = !LEDG;
}
