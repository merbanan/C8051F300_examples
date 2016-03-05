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

/** Simple C8051F300 sdcc uart command test
 * The mcu parses commands sent over the serial port.
 * The commands will control gpio pins
 * "l#<enter>" will set the P0 # bit
 * "L#<enter>" will unset the P0 # bit
 *
 * Compile with "sdcc uart_loop_back.c" and flash uart_loop_back.ihx
 *
 */

#include <C8051F300.h>

#define BUFFER_SIZE 32

unsigned char uart_rx_buffer[BUFFER_SIZE];
unsigned char rx_buf_idx = 0;
__bit tx_busy = 0;				// Uart transmission in progress
unsigned char command_ready = 0;
unsigned char command = 0;
unsigned char command_argument = 0;


void send_byte(unsigned char byte)
{
	while(tx_busy);				// Wait until tx queue is free
	tx_busy = 1;				// Set uart tx busy
	SBUF0=byte;					// Transmit byte
}

void send_string(unsigned char *string)
{
	while(*string)
		send_byte(*string++);	// Send string 1 byte at a time
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

	/* Configure interrupts */
	IP = 0x10;					// Set UART interrupts to high priority
	ES0 = 1;					// Enable UART0 interrupts
	EA = 1;						// Enable global interrupts

	send_string("Ready for command test:\r\n");
	while (1) {					// Loop forever
 		if (command_ready) {
			switch (command) {
				case 'l':
					P0 |= 1<< (command_argument-'0');
					break;
				case 'L':
					P0 &= ~(1<< (command_argument-'0'));
					break;
				default:
					send_string("Error\r\n");
			}
 			command_ready = 0;
 		}
	}
}

void UART0_ISR (void) __interrupt 4
{
	if (RI0) {
		RI0 = 0;				// Clear interrupt
		uart_rx_buffer[rx_buf_idx] = SBUF0;	// Add received byte to buffer
		tx_busy = 1;			// Set uart tx busy
		SBUF0 = uart_rx_buffer[rx_buf_idx];	// Send back the received byte
		rx_buf_idx++;

		/* Parse message */
		if (rx_buf_idx >=3) {
			rx_buf_idx = 0;
			command_ready = 1;
			command = uart_rx_buffer[0];
			command_argument = uart_rx_buffer[1];
		}
	}
	if (TI0) {
		TI0 = 0;				// Clear interrupt
		tx_busy = 0;			// UART ready to transmit next byte
	}
}
