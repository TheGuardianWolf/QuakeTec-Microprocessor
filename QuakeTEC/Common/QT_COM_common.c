/*
 * Includes
 */
#include "QT_COM_common.h"

/*
 * Public functions
 */

/**
 * Finds the maximum value in an array.
 */
uint16_t QT_COM_max(uint16_t *values, uint16_t length) {
    int i;
    uint16_t currentMax = values[0];

    for (i = 1; i < length; i++) {
        if (values[i] > currentMax) {
            currentMax = values[i];
        }
    }

    return currentMax;
}

uint16_t QT_COM_maxByteArray(byte *values, uint16_t length) {
    int i;
    uint16_t currentMax = (((uint16_t) values[0])<<8) + values[1];
    uint16_t temp = 0;

    for (i = 2; i < length -1; i++) {
        temp = (((uint16_t) values[i])<<8) + values[i+1];
        if (temp > currentMax) {
            currentMax = temp;
        }
        i++;
    }

    return currentMax;
}

uint16_t QT_COM_minByteArray(byte *values, uint16_t length) {
    int i;
    uint16_t currentMin = (((uint16_t) values[0])<<8) + values[1];
    uint16_t temp = 0;

    for (i = 2; i < length - 1; i++) {
        temp = (((uint16_t) values[i])<<8) + values[i+1];
        if (temp > currentMin) {
            currentMin = temp;
        }
        i++;
    }

    return currentMin;
}


/**
 * Finds the minimum value in an array.
 */
uint16_t QT_COM_min(uint16_t *values, uint16_t length) {
    int i;
    uint16_t currentMin = values[0];

    for (i = 1; i < length; i++) {
        if (values[i] < currentMin) {
            currentMin = values[i];
        }
    }

    return currentMin;
}

void binary_print(int value) {
    int r, temp;
    int i=0;
    temp = value;
    for (i=0; i<16; i++) {
        r = BIT0 & temp;
        if (r > 0) {
            P1DIR |= BIT1;
            P1OUT |= BIT1;
        } else {
            P1DIR |= BIT0;
            P1OUT |= BIT0;
        }
        temp = temp>>1;
        __delay_cycles(1000000);
        P1OUT = 0;
        __delay_cycles(1000000);
    }
    __delay_cycles(3000000);
}
