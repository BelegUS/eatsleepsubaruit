#include <avr/io.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <string.h>
#include "uart.h"

/**********************************************************************************/
/* Function generates command buffer for querying ECU parameters		          */
/*																				  */
/* @param unsigned char msb - Most Significant Byte address of ECU parameter      */
/* @param unsigned char lsb - Least Significant Byte address of ECU parameter     */
/* @param char* buffer4 - Buffer to be filled with command data. Size of 4 chars  */
/* @returns char* buffer4 - Filled with query command buffer. Size of 4 chars     */
/**********************************************************************************/
char* queryEcuCommand(unsigned char msb, unsigned char lsb, char* buffer4)
{

	buffer4[0]=(char)0x78;
	buffer4[1]=(char)msb;
	buffer4[2]=(char)lsb;
	buffer4[3]=(char)0x00;
	
	return buffer4;
}

/**********************************************************************************/
/* Function generates command buffer for querying ECU ROM ID			          */
/*																				  */
/* @param char* buffer4 - Buffer to be filled with query command. Size of 4 chars */
/* @returns char* buffer4 - Filled with query command buffer. Size of 4 chars     */
/**********************************************************************************/
char* queryEcuRomIdCommand(char* buffer4)
{

	buffer4[0]=(char)0x00;
	buffer4[1]=(char)0x46;
	buffer4[2]=(char)0x48;
	buffer4[3]=(char)0x49;
	
	return buffer4;
}

/**********************************************************************************/
/* Function generates command buffer to stop querying ECU ROM ID		          */
/*																				  */
/* @param char* buffer4 - Buffer to be filled with query command. Size of 4 chars */
/* @returns char* buffer4 - Filled with query command buffer. Size of 4 chars     */
/**********************************************************************************/
char* stopQueryEcuCommand(char* buffer4)
{

	buffer4[0]=(char)0x12;
	buffer4[1]=(char)0x00;
	buffer4[2]=(char)0x00;
	buffer4[3]=(char)0x00;
	
	return buffer4;
}

/**********************************************************************************/
/* Function gets chars from Bluetooth Module, sanity checks them, then			  */
/* fills passed buffer with Command, MSB, LSB and "Finisher" of Subaru Query	  */
/* requested by BT device                                                         */
/*																				  */
/* @param char* buffer4 - Buffer to be filled with query command. Size of 4 chars */
/* @returns integer value 1 if correct query command was received, 0 otherwise    */
/**********************************************************************************/
int getSubaruQueryFromBluetooth(char* buffer4)
{
	unsigned int tempChar = UART_NO_DATA;	//Helper temp buffer for chars. Empty at start
	
	/*Get chars from BT until data arrives
	We do not worry about being stuck here, BT
	will feed us with data.
	This data will be sanity checked for being
	the begining of command*/
	while(tempChar == UART_NO_DATA) {
		tempChar = uart1_getc();
	}
	/* Check if it is correct begining char (STX)*/
	if(tempChar != (char)0x02) {
		return 0;
	}
	tempChar = UART_NO_DATA;	//Clear the temp buffer.
	
	/* Get chars from BT until data arrives
	We do not worry again, it will arrive.
	This data will be treated as Command byte of Subaru ECU Query*/
	while(tempChar == UART_NO_DATA) {
		tempChar = uart1_getc();
	}
	buffer4[0] = (char)tempChar;	//Store Command in buffer
	tempChar = UART_NO_DATA;	//Clear the temp buffer.	
	
	/* Get chars from BT until data arrives
	We do not worry again, it will arrive.
	This data will be treated as MSB of Subaru ECU Query*/
	while(tempChar == UART_NO_DATA) {
		tempChar = uart1_getc();
	}
	buffer4[1] = (char)tempChar;	//Store MSB in buffer
	tempChar = UART_NO_DATA;	//Clear the temp buffer.
	
	/* Get chars from BT until data arrives
	We do not worry again, it will arrive.
	This data will be treated as LSB of Subaru ECU Query*/
	while(tempChar == UART_NO_DATA) {
		tempChar = uart1_getc();
	}
	buffer4[2] = (char)tempChar;	//Store LSB in buffer
	tempChar = UART_NO_DATA;	//Clear the temp buffer.
	
	buffer4[3] = (char)0x00;	//Store byte marking end of query in buffer	
	
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
	int startQueryingSubaru = 0;	//Buffer for value to check if program can start querying SSM-1
	unsigned int s = UART_NO_DATA;	//Buffer for things from Subaru
	unsigned char queryBuffer[4];	//Buffer for complete Subaru Query

	uart_init(UART_BAUD_SELECT(1953,F_CPU));	//UART0, Subaru SSM-1
	uart1_init(UART_BAUD_SELECT(9600,F_CPU));	//UART1, Bluetooth Module
	
	/* Flush UART buffers from any kind of trash that can be there */
	uart_flush();
	uart1_flush();
	
	sei(); 
	
    while(1)
    {

		/* Wait for first correct query addresses from Bluetooth */
		do 
		{
			startQueryingSubaru = getSubaruQueryFromBluetooth( queryBuffer );
		} while(startQueryingSubaru == 0);
		uart1_flush();
		startQueryingSubaru = 0;
		
		/* Query Subaru SSM until it responds */
		do {
			uart_putsubaru( queryBuffer );
			_delay_ms(3);
		} while(uart_available() == 0 && uart1_available() < 5);
		
		/* Receive and transmit collected bytes of data from car until there is new command in BT receive buffer */
		do {
			s = uart_getc();
			if(s == UART_NO_DATA || s & UART_FRAME_ERROR || s & UART_OVERRUN_ERROR || s & UART_BUFFER_OVERFLOW) {
				//Do nothing. Exception handling can be done here.
			} else {
				uart1_putc(s);	//Transmit byte from Car via BT
			}
			s = UART_NO_DATA;
		} while(uart1_available() < 5);

	}
}