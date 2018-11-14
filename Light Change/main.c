#include <stdio.h>
#include <msp430.h> 


/**
 * hello.c
 */
int main(void)
{
    int i;

	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer

    P2DIR &= ~BIT0;             // P2.0 is the input
    P1DIR |= BIT0;

    PM5CTL0 &= ~LOCKLPM5;

    int maximumInt = 1000;
    int intensity = 0;

    while(1) {
        if(!(P2IN & BIT0)) {
            intensity = (intensity + 1) % maximumInt;
        }

        for(i = 0; i < maximumInt - intensity; i++)
            __delay_cycles(10);

        P1OUT |= BIT0;

        for(i = 0; i < intensity; i++)
            __delay_cycles(10);
        P1OUT &= ~BIT0;
    }
}
