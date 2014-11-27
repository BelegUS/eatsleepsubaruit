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


char* queryEcuCommand(unsigned char msb, unsigned char lsb, char* buffer4)
{

	buffer4[0]=(char)0x78;
	buffer4[1]=(char)msb;
	buffer4[2]=(char)lsb;
	buffer4[3]=(char)0x00;
	
	return buffer4;
}

char* queryRomIdCommand(char* buffer4)
{

	buffer4[0]=(char)0x00;
	buffer4[1]=(char)0x46;
	buffer4[2]=(char)0x48;
	buffer4[3]=(char)0x49;
	
	return buffer4;
}

int getQueryAddressBytes(char* buffer2)
{
	unsigned int tempChar = UART_NO_DATA;	//Helper temp buffer for chars. Empty at start
	
	/*Get chars from BT until correct begining char (STX)
	We do not worry about being stuck here, Telephone
	will feed us with data*/
	while(tempChar != (char)0x02) {
		tempChar = uart1_getc();
	}
	tempChar = UART_NO_DATA;	//Clear the temp buffer.
	
	/* Get chars from BT until data arrives
	We do not worry again, it will arrive.
	This data will be treated as MSB of Subaru ECU Query*/
	while(tempChar == UART_NO_DATA) {
		tempChar = uart1_getc();
	}
	buffer2[0] = (char)tempChar;	//Store MSB in buffer
	tempChar = UART_NO_DATA;	//Clear the temp buffer.
	
	/* Get chars from BT until data arrives
	We do not worry again, it will arrive.
	This data will be treated as LSB of Subaru ECU Query*/
	while(tempChar == UART_NO_DATA) {
		tempChar = uart1_getc();
	}
	buffer2[1] = (char)tempChar;	//Store LSB in buffer
	tempChar = UART_NO_DATA;	//Clear the temp buffer.	
	
	/* Get chars from BT until data arrives
	We do not worry again, it will arrive.
	This char (ETB) will be used to sanity check
	the whole chunk transmission*/
	while(tempChar == UART_NO_DATA) {
		tempChar = uart1_getc();
	}
	if(tempChar == (char)0x17) {
		return 1;	//Everything went OK, we can use data from Buffer
	} else {
		return 0;	//Something went wrong, data in Buffer isn't OK
	}
	
} 


int main(void)
{
	unsigned int c = UART_NO_DATA;	//Buffer for things from Console
	unsigned int s = UART_NO_DATA;	//Buffer for things from Subaru
	unsigned char queryBuffer[4];	//Buffer for complete Subaru Query
	unsigned char addressBuffer[2];	//Buffer for MSB and LSB to create Subaru Query

	uart_init(UART_BAUD_SELECT(1953,F_CPU));	//UART0, Subaru SSM-1
	uart1_init(UART_BAUD_SELECT(9600,F_CPU));	//UART1, Bluetooth Module
	
	sei(); 
	
    while(1)
    {
		/* Flush UART buffers */
		uart_flush();
		uart1_flush();
		
		/* Wait for synchronization (SYN) char from Bluetooth */
		while(c != (char)0x16)
		{
			c = uart1_getc();
		}

		uart_putsubaru( testBuffer );		//Zabezpiecz to - co jeœli auto nie odpowie?						
	
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
	
			c = UART_NO_DATA;
		}

    }
