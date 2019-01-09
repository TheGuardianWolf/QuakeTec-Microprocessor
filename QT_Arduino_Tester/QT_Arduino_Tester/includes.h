/*
 * includes.h
 *
 * Created: 26/09/2018 
 *  Author: Team 1 
 */ 
#define F_CPU 16000000

#include <avr/io.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <avr/interrupt.h>
#include <util/delay.h> //Remove in final version
#include <stdbool.h>

/*UART Functions*/
void uart_initialise(uint16_t UBRR_VALUE);
void uart_transmit(uint8_t data);
void uart_flush(void);