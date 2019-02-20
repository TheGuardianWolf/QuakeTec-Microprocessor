/*
 * QT_DAC.c
 *
 *  Created on: 16/01/2019
 *      Author: james
 */

#include "spilib.h"

static uint16_t data;

/**
 * This method sends a particular bit value to the DAC
 *
 * This method blocks until all other transmissions have finished.
 **/
void QT_DAC_setOutputValue(uint16_t value) {
    data = value;
    while(!QT_SPI_transmit((byte *) &data, 2, &DAC));
}
