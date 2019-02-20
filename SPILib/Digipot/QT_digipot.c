/*
 * QT_digipot.c
 *
 *  Created on: 16/01/2019
 *      Author: james
 */

#include <stdlib.h>

#include "driverlib.h"
#include "SpiLib/QT_SPI_SpiLib.h"

static uint16_t registerValue;

// These variables control the shifting of data into the digipot register
// reg = reg & ~MASK | ((data << SHIFT) & MASK)

#define CONTROL_MASK 0x3c00
#define CONTROL_SHIFT 10

#define DATA_MASK 0x3ff

#define DIGIPOT_RDY_PORT GPIO_PORT_P2
#define DIGIPOT_RDY_PIN GPIO_PIN7

static void setReady(bool high) {
    if (high) {
        GPIO_setOutputHighOnPin(DIGIPOT_RDY_PORT, DIGIPOT_RDY_PIN);
    } else {
        GPIO_setOutputLowOnPin(DIGIPOT_RDY_PORT, DIGIPOT_RDY_PIN);
    }
}

/**
 * This sends the whole register to the digipot, control and data bits.
 *
 * If the SPI line is busy this method waits until it is free.
 **/
static void syncRegister() {
    // TODO validate timings with o-scope
    // We need to make this so the method does not block for extended periods of time.

    setReady(true);
    while(!QT_SPI_transmit((byte *) &registerValue, 2, &DIGIPOT, NULL));
    while(!QT_SPI_isDataSent());
    setReady(false);
}

/**
 * This sets the control bits for the Digipot. If data is not ready to send this
 * method will block until the data starts to send.
 *
 * Only the lower 4 bits of data will be sent.
 */
void QT_DIGIPOT_setControlBits(uint8_t controlBits) {
    registerValue &= ~CONTROL_MASK;
    registerValue |= (controlBits << CONTROL_SHIFT) & CONTROL_MASK;

    syncRegister();
}

/**
 * This sets the data bits for the Digipot. If data is not ready to send this
 * method will block until the data starts to send.
 */
void QT_DIGIPOT_setDataBits(uint8_t dataBits) {
    registerValue &= ~DATA_MASK;
    registerValue |= dataBits & DATA_MASK;

    syncRegister();
}
