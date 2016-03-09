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

/** Timer us counter
 * Sysclock = 24500000
 * Timer 0 clock = sysclock / 4 = 24500000 / 4 = 6125000
 * 1 us (T0 clock) -> 6125000 / 1000000 = 6.125
 * So 6.125 cycles per us
 */

#include <C8051F300.h>

#define BUFFER_SIZE 120

__idata unsigned char uart_rx_buffer[BUFFER_SIZE];
unsigned char rx_buf_idx = 0;
volatile __bit tx_busy = 0;				// Uart transmission in progress
__bit cmd_ready = 0;
unsigned char cmd = 0;
unsigned char cmd_len = 0;
unsigned char cmd_arg = 0;

__bit field_on = 0;
__bit modulate_field = 0;
__bit skip_cycle = 0;
unsigned char field_idx = 2;
unsigned char dummy = 0;

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
	int i;
	PCA0MD &= ~0x40;			// Disable the watchdog timer

	/* Clock init */
	OSCICN = 0x17;				// Enable internal oscillator (24.5 MHz)
								// Set sysclock to internal oscillator
	RSTSRC = 0x04;				// Enable missing clock detector

	/* Port/Crossbar init */
	P0MDOUT = 0x10;				// Set port P0.4 (Uart tx) to push-pull
	XBR0 = 0xCF;				// UART is always on P0.4 and P0.5
	XBR1 = 0x02;				// Enable UART RX and TX
//	XBR1 = 0x03;				// Enable UART RX and TX
	XBR2 = 0x40;				// Enable the crossbar

	/* Configure UART for a baud rate of 115200 */
	SCON0 = 0x10;				// Enable UART reception
	CKCON = 0x10;				// Timer 1 uses the sysclock / 4
	TH1 = 0x96;					// 115200 baud rate reload value
	TMOD = 0x20;				// Select 8-bit counter for Timer 1
	TR1 = 1;					// Enable Timer 1

	/* Configure Timer 0 */
	TMOD |= 0x01;				// Set timer mode 1 (16-bit)
//	CKCON |= 0x02;				// Set timer clock to sysclock / 48
	CKCON |= 0x01;				// Set timer clock to sysclock / 4
	TH0 = 0xFF;					// Init timer 0 high byte
	TL0 = 0xFF;					// Init timer 0 low byte
	ET0 = 1;					// Enable Timer 0 interrupts
	TR0 = 1;					// Enable Timer 0

	/* Configure interrupts */
	IP = 0x10;					// Set UART interrupts to high priority
	ES0 = 1;					// Enable UART0 interrupts
	EA = 1;						// Enable global interrupts

	P0_6 = 0;
	P0_7 = 0;
	send_string("Ready for command test:\r\n");
	while (1) {					// Loop forever
 		if (cmd_ready) {
			switch (cmd) {
				case 'l':
					P0 |= 1<< (cmd_arg-'0');
					break;
				case 'L':
					P0 &= ~(1<< (cmd_arg-'0'));
					break;
				case 't':
					TR0 = cmd_arg - '0';
					break;
				case 'f':
					field_on = cmd_arg - '0';
					break;
				case 'o':
					send_string("Buffer:\r\n");
					for (i=0 ; i<cmd_len-'0' ; i++) {
						send_byte(uart_rx_buffer[i+field_idx]);
					}
					send_string("\r\nEnd of buffer\r\n");
					field_idx = 2;					
					modulate_field = 1;
					break;
				default:
					send_string("Error\r\n");
			}
 			cmd_ready = 0;
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

		/* Parse message */
		if ((uart_rx_buffer[rx_buf_idx] == '\n') || (uart_rx_buffer[rx_buf_idx] == '\r')) {
			rx_buf_idx = 0;
			cmd_ready = 1;
			cmd = uart_rx_buffer[0];
			cmd_len = uart_rx_buffer[1];
			cmd_arg = uart_rx_buffer[2];
		} else {
			rx_buf_idx++;
			//FIXME make this cyclical to prevent memory overwrite
		}
	}
	if (TI0) {
		TI0 = 0;				// Clear interrupt
		tx_busy = 0;			// UART ready to transmit next byte
	}
}



void TIMER0_ISR (void) __interrupt 1
{
	/* These timer values should result in a 4us periodic interrupt */
	TH0 = 0xFF;					// Init timer 0 high byte
	TL0 = 0xF2;					// Init timer 0 low byte

	if (modulate_field) {
		field_on = uart_rx_buffer[field_idx+1] - '0';
//		send_byte(field_on + '0');
//		field_on = 0;
		if (uart_rx_buffer[field_idx] == '0') {
			if (skip_cycle)
				field_idx += 2;
//			send_byte('|');
			if (field_idx >= cmd_len -'0' + 2) {
//				send_byte('c');
				field_idx = 2;
				modulate_field = 0;
				field_on = 0;
			}
		} else
			uart_rx_buffer[field_idx] = uart_rx_buffer[field_idx] - 1;
	} else {
		/* The else path needs to take the same amount of time as the main path */
		dummy = uart_rx_buffer[field_idx];
		dummy++;
	}

	skip_cycle = !skip_cycle;

	if (field_on == 1)
//		P0_0 = !P0_0;
		P0_4 = !P0_4;
	else
//		P0_0 = 0;
		P0_4 = 0;
}
