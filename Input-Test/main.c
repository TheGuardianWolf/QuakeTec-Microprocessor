#include <msp430.h> 

/**
 * main.c
 */
int main(void)
{
    // Stop watchdog timer
	WDTCTL = WDTPW | WDTHOLD;

	// Set pins as inputs
	//P3DIR &= ~BIT1; // P3.1
	//P3DIR &= ~BIT0; // P3.0
	//P5DIR &= ~BIT1; // P5.1
	P5DIR &= ~BIT0; // P5.0

	// Enable pull-up resistor to force voltage to high when pin is disconnected
	P5REN |= BIT0;
	P5OUT |= BIT0;

	// Set LED as output
	P1DIR |= BIT1; // P1.1

	// Clear the LED
	P1OUT &= ~BIT1;

	// Disable the GPIO power-on default high-impedance mode to activate
	// previously configured port settings
	PM5CTL0 &= ~LOCKLPM5;
	
	// Make LED 2 mirror input - switch moves to next pin
	while (1) {
	    // If input pin is high, turn on LED
	    if (P5IN & BIT0) {
	        P1OUT |= BIT1;
	    } else { // turn off the LED
	        P1OUT &= ~BIT1;
	    }
	}

	return 0;
}
