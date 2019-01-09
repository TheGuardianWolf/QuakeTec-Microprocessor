/*
 * QT_Arduino_Tester.c
 *
 * Created: 18/11/2018 1:12:56 PM
 */ 

#ifndef F_CPU
#define F_CPU 16000000
#endif

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "includes.h"

#define ACK 0x7E
#define BAUD 9600
#define MYUBRR F_CPU/16/BAUD-1

void spi_init_slave (void)
{
	DDRB=(1<<4);                                  //MISO as OUTPUT
	SPCR=(1<<SPE);                                //Enable SPI
	SPCR|=(1<<CPOL)|(1<<CPHA);
}

//Function to send and receive data
unsigned char spi_tranceiver (unsigned char data)
{
	SPDR = data;                                  //Load data into buffer
	while(!(SPSR & (1<<SPIF) ));                  //Wait until transmission complete
	return(SPDR);                                 //Return received data
}

int main(void)
{
	uart_initialise(MYUBRR);
	spi_init_slave();                             //Initialize slave SPI
	uint8_t data = 0;
	DDRB  = 0x00;                                 //Initialize PORTA as INPUT
	PORTB = 0xFF;                                 //Enable Pull-Up Resistors
	
	uint8_t data_array[20];

	uint8_t i = 0;
	
	while(1)
	{
		
		uint8_t old_byte = 0;
		for(i = 0; i<20; i++){
			data = SPDR;
 			PORTB ^= (1<<0);
			if(old_byte != data){
				data_array[i] = data;
				data_array[i+1] = '\0';
				old_byte = data;
			}
		}		
		
		_delay_ms(500);
		
		if(){
			uart_transmit(13);
			uart_transmit(10);
			uint8_t n = 0;
			while(data_array[n] != '\0'){
				uart_transmit(data_array[n]);
				_delay_ms(20);                            //Wait
				n++;
			}
			uart_transmit(13);
			uart_transmit(10);
		}
		
		
//		itoa(data, buffer, 10);                   //Convert integer into string
// 		if(old_byte != data){

// 			uart_transmit(data);                    //Display received data
// 			old_byte = data;
// 		}

	}
}