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

/** Simple C8051F300 sdcc timer example
 * Enable 2 timers and let them generate interrupts.
 * In the interrupt service routine toggle the led.
 *
 * Compile with "sdcc timer.c" and flash timer.ihx
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
	OSCICN = 0x17;				// Enable internal oscillator (24.5 MHz)
								// Set sysclock to internal oscillator
	RSTSRC = 0x04;				// Enable missing clock detector

	/* Port/Crossbar init */
	XBR0 = 0xFF;				// Skip all pins in crossbar selection
	XBR2 = 0x40;				// Enable the crossbar

	/* Configure Timer 0 and 1 */
	TMOD = 0x11;				// Set timer mode 1 (16-bit)
	CKCON = 0x02;				// Set timer clock to sysclock / 48
	TH0 = 0x1F;					// Init timer 0 high byte
	TL0 = 0x00;					// Init timer 0 low byte
	TH1 = 0x7F;					// Init timer 1 high byte
	TL1 = 0x00;					// Init timer 1 low byte
	ET0 = ET1 = 1;				// Enable Timer 0 and 1 interrupts
	TR0 = TR1 = 1;				// Enable Timer 0 and 1
	EA = 1;						// Enable global interrupts

	while (1);
}

void TIMER0_ISR (void) __interrupt 1
{
	TH0 = 0x1F;					// Init timer 0 high byte
	TL0 = 0x00;					// Init timer 0 low byte
	LEDY = !LEDY;
}

void TIMER1_ISR (void) __interrupt 3
{
	TH1 = 0x7F;					// Init timer 1 high byte
	TL1 = 0x00;					// Init timer 1 low byte
	LEDG = !LEDG;
}
