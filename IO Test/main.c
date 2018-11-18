#include <msp430.h>

/*
 * Define type for bytes
 **/
typedef unsigned char BYTE;

/*
 * Define struct for pins
 **/
typedef struct Pin {
    BYTE port;
    BYTE mask;
} Pin_t;

/*
 * Sequence of pins that are probed by the input test
 **/
const Pin_t inputSequence[] = {
                               {3, BIT1},
                               {3, BIT0},
                               {1, BIT3},
                               {5, BIT1},
                               {5, BIT0}
};

/*
 * Sequence of pins that get turned on by the output test
 */
const Pin_t outputSequence[17] = {
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

const Pin_t theSwitch = {2, BIT0};

const Pin_t LED = {1, BIT0};

/**
 * Returns the address of the direction register for a given port number
 **/
volatile BYTE *getDirection(BYTE port) {
    switch(port) {
    case 1: return &P1DIR;
    case 2: return &P2DIR;
    case 3: return &P3DIR;
    case 4: return &P4DIR;
    case 5: return &P5DIR;
    }

    return 0;
}

volatile BYTE *getOut(BYTE port) {
    switch(port) {
    case 1: return &P1OUT;
    case 2: return &P2OUT;
    case 3: return &P3OUT;
    case 4: return &P4OUT;
    case 5: return &P5OUT;
    }

    return 0;
}

volatile BYTE *getIn(BYTE port) {
    switch(port) {
    case 1: return &P1IN;
    case 2: return &P2IN;
    case 3: return &P3IN;
    case 4: return &P4IN;
    case 5: return &P5IN;
    }

    return 0;
}

/**
 * Sets all input pins used by the microcontroller.
 */
void setAllInput() {
    unsigned int i;
    BYTE port, mask;

    // Set input pins
    for (i = 0; i < sizeof(inputSequence)/sizeof(BYTE); i++) {
        port = inputSequence[i].port;
        mask = inputSequence[i].mask;

        // Set direction to input
        *getDirection(port) &= ~mask;
    }
}

/*
 * Enable the output and clear all of the output pins
 */
void setAllOutput() {
    BYTE port, mask;
    unsigned int i;

    for(i = 0; i < 17; i++) {
        port = outputSequence[i].port;
        mask = outputSequence[i].mask;

        // Set pin as output
        *getDirection(port) |= mask;

        // Clear noise
        *getOut(port) &= ~mask;
    }
}

/*
 * Flash the pin and the LED
 */
void flash(volatile BYTE *pin, BYTE mask) {

    *pin ^= mask;
    *getOut(LED.port) ^= LED.mask;
    __delay_cycles(100000);
}

/**
 * Pin is a pointer to the port that will be flashed on.
 * Mask is a mask for the buttons. This function will block until switch is released.
 **/
void waitAndFlash(volatile BYTE *pin, BYTE mask) {

    // Flash until the switch is pressed
    BYTE inputValues = *getIn(theSwitch.port);

    while (inputValues & theSwitch.mask) {
        flash(pin, mask);
    }

    // Flash until the switch is released
    while ((~inputValues) & theSwitch.mask) {
        flash(pin, mask);
    }
}

/*
 * If the pin is high, set the LED high and vice versa
 */
void link(volatile BYTE *pin, BYTE mask) {

    // Check if the pin is high
    if (*pin & mask) {
        *getOut(LED.port) |= LED.mask;
    } else {
        *getOut(LED.port) |= LED.mask;
    }
    __delay_cycles(10000);
}

/**
 * Pin is a pointer to the port that will be flashed on.
 * Mask is a mask for the buttons. This function will block until switch is released.
 **/
void waitAndLink(volatile BYTE *pin, BYTE mask) {

    BYTE switchValues = *getIn(theSwitch.port);

    // Link until the switch is pressed
    while (switchValues & theSwitch.mask) {
        link(pin, mask);
    }

    // Link until the switch is released
    while ((~switchValues) & theSwitch.mask) {
        link(pin, mask);
    }
}

/**
 * Links the next input pin in inputSequence to the LED every time the switch is pressed
 */
void inputTest() {
    unsigned int i = 0;
    BYTE port, mask;

    for (i = 0; i < sizeof(inputSequence)/sizeof(BYTE); i++) {

        // Clear the LED
        *getOut(LED.port) &= ~LED.mask;

        port = inputSequence[i].port;
        mask = inputSequence[i].mask;

        waitAndLink(getIn(port), mask);
    }
}

/**
 * Flashes the next output pin in outputSequence every time the switch is pressed
 */
void outputTest() {
    unsigned int i = 0;
    BYTE port, mask;

    for (i = 0; i < 17; i++) {

        if (i != 0) {

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

/**
 * Sets up the system
 */
void config() {
    // Stop watchdog timer
    WDTCTL = WDTPW | WDTHOLD;

    // Clear some bit to make everything work
    PM5CTL0 &= ~LOCKLPM5;

    // Turn on the manual switch as an input
    *getDirection(theSwitch.port) &= ~theSwitch.mask;

    // Set up LED as a debug output and clear the LED
    *getDirection(LED.port) |= LED.mask;
    *getOut(LED.port) &= ~LED.mask;

    // Set up the inputs and outputs for testing
    setAllInput();
    setAllOutput();
}

/**
 * main.c
 */
int main(void)
{
    config();

    // Run the tests
    //inputTest();
    outputTest();

    return 0;
}
