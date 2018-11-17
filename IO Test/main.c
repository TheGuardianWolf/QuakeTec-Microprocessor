#include <msp430.h>

void flash(volatile unsigned char *pin, char mask) {
    *pin |= mask;
    P1OUT |= BIT0;
    __delay_cycles(100000);
    *pin &= ~mask;
    P1OUT &= ~BIT0;
    __delay_cycles(100000);
}

/**
 * pin is a pointer to the port that will be flashed on.
 * mask is a mask for the buttons. This function will block until P2.0
 **/
void waitAndFlash(volatile unsigned char *pin, unsigned char mask) {
    //wait for all bits to be 0
    while(P2IN & BIT0) {
        flash(pin, mask);
    }

    //wait for all bits to be 1
    while((~P2IN) & BIT0) {
        flash(pin, mask);
    }
}

/**
 * Returns the address of the direction register for a given port number
 **/
volatile unsigned char *getDirection(unsigned char port) {
    switch(port) {
    case 1: return &P1DIR;
    case 2: return &P2DIR;
    case 3: return &P3DIR;
    case 4: return &P4DIR;
    case 5: return &P5DIR;
    }

    return 0;
}

volatile unsigned char *getSelection0(unsigned char port) {
    switch(port) {
    case 1: return &P1SEL0;
    case 2: return &P2SEL0;
    case 3: return &P3SEL0;
    case 4: return &P4SEL0;
    case 5: return &P5SEL0;
    }

    return 0;
}

volatile unsigned char *getSelection1(unsigned char port) {
    switch(port) {
    case 1: return &P1SEL1;
    case 2: return &P2SEL1;
    case 3: return &P3SEL1;
    case 4: return &P4SEL1;
    case 5: return &P5SEL1;
    }

    return 0;
}

volatile unsigned char *getOut(unsigned char port) {
    switch(port) {
    case 1: return &P1OUT;
    case 2: return &P2OUT;
    case 3: return &P3OUT;
    case 4: return &P4OUT;
    case 5: return &P5OUT;
    }

    return 0;
}

volatile unsigned char *getIn(unsigned char port) {
    switch(port) {
    case 1: return &P1IN;
    case 2: return &P2IN;
    case 3: return &P3IN;
    case 4: return &P4IN;
    case 5: return &P5IN;
    }

    return 0;
}

typedef struct {
    unsigned char port;
    unsigned char mask;
} Step;

// This is the sequence of pins that gets turned on by the output test.
const Step outputSequence[17] = {
    { 3, BIT2 },

    { 1, BIT2 },
    { 1, BIT1 },

    { 2, BIT7 },
    { 2, BIT6 },
    { 2, BIT5 },
    { 2, BIT4 },

    { 4, BIT7 },
    { 4, BIT6 },
    { 4, BIT5 },
    { 4, BIT4 },
    { 4, BIT3 },
    { 4, BIT2 },
    { 4, BIT1 },
    { 4, BIT0 },

    { 3, BIT4 },
    { 3, BIT3 }
};

// Enable the output and clear all of the output pins
void setAllOutput() {
    unsigned char port, mask;
    unsigned int i;

    for(i = 0; i < 17; i++) {
        port = outputSequence[i].port;
        mask = outputSequence[i].mask;

        // Set function as IO
        *getSelection0(port) &= ~mask;
        *getSelection1(port) &= ~mask;

        // Set direction as output
        *getDirection(port) |= mask;

        // Clear bit
        *getOut(port) &= ~mask;
    }
}

void outputTest() {
    unsigned int i = 0;
    unsigned char port;
    unsigned char mask;

    for(i = 0; i < 17; i++) {
        if(i != 0) {
            // Clear the old output
            port = outputSequence[i - 1].port;
            mask = outputSequence[i - 1].mask;

            *getOut(port) &= ~mask;
        }

        port = outputSequence[i].port;
        mask = outputSequence[i].mask;

        waitAndFlash(getOut(port), mask);
    }
}

void inputTest() {

}

/**
 * main.c
 */
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer

    PM5CTL0 &= ~LOCKLPM5;

    // Setup and clear the outputs
    setAllOutput();

    // Turn on the button P2.0 as an input
    P2DIR &= ~BIT0;

    // Setup P1.0 as a debug output
    P1DIR |= BIT0;
    P1OUT &= ~BIT0;

    outputTest();

    return 0;
}
