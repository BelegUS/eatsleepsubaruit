/*
 * SSM1C.c
 *
 * Created: 2014-06-25 22:54:39
 *  Author: BelegUS
 */ 

#include <avr/io.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <string.h>
#include "uart.h"


char* readFromECU(unsigned char msb, unsigned char lsb, char* buffer)
{

	buffer[0]=(char)0x78;
	buffer[1]=(char)msb;
	buffer[2]=(char)lsb;
	buffer[3]=(char)0x00;
	
	return buffer;
}

char* readRomId(char* buffer)
{

	buffer[0]=(char)0x00;
	buffer[1]=(char)0x46;
	buffer[2]=(char)0x48;
	buffer[3]=(char)0x49;
	
	return buffer;
}


int main(void)
{
	unsigned int c = UART_NO_DATA;	//Buffer for things from Console
	unsigned int s = UART_NO_DATA;	//Buffer for things from Subaru
	unsigned char standardBuffer[4];
	unsigned char bigBuffer[60];
	char testBuffer[] = { (unsigned char)0x78, (unsigned char)0x00, (unsigned char)0x09, (unsigned char)0x00 };
	unsigned char testBuffer2[] = { 'c', 'h', 'a', 'r' };
	//uart1_init(UART_BAUD_SELECT(1953,F_CPU));	//UART1, Komputer
	uart1_init(UART_BAUD_SELECT(9600,F_CPU));	//UART1, Telefon BT
	uart_init(UART_BAUD_SELECT(1953,F_CPU));	//UART0, Samochód
	sei(); 
    while(1)
    {
 /*
         * Get received character from ringbuffer
         * uart_getc() returns in the lower byte the received character and 
         * in the higher byte (bitmask) the last receive error
         * UART_NO_DATA is returned when no data is available.
         *
         */
		uart_flush();
		uart1_flush();
		
		while(c == UART_NO_DATA)
		{
			c = uart1_getc();
		}
            /*
             * new data available from UART
             * check for Frame or Overrun error
             */
            if ( c & UART_FRAME_ERROR )
            {
                /* Framing Error detected, i.e no stop bit detected */
                uart1_puts("PC UART Frame Error: ");
            }
            if ( c & UART_OVERRUN_ERROR )
            {
                /* 
                 * Overrun, a character already present in the UART UDR register was 
                 * not read by the interrupt handler before the next character arrived,
                 * one or more received characters have been dropped
                 */
                uart1_puts("PC UART Overrun Error: ");
            }
            if ( c & UART_BUFFER_OVERFLOW )
            {
                /* 
                 * We are not reading the receive buffer fast enough,
                 * one or more received character have been dropped 
                 */
                uart1_puts("PC Buffer overflow error: ");
            }
			else{
				uart_putsubaru( testBuffer );								
	
			while(1) {
					while(s == UART_NO_DATA)
					{
						s = uart_getc();
					}	
				
			if ( s & UART_FRAME_ERROR )
            {
                uart1_puts("Car UART Frame Error: ");
            }
            if ( s & UART_OVERRUN_ERROR )
            {
                uart1_puts("Car UART Overrun Error: ");
            }
            if ( s & UART_BUFFER_OVERFLOW )
            {
                //uart1_puts("Car Buffer overflow error: ");
            }
			if ( s == UART_NO_DATA || s == 0)
			{
				//Do nothing
			}
			else {
				uart1_putc(s);
			}
			s = UART_NO_DATA;
				

			}
		}
			c = UART_NO_DATA;
		}

    }
