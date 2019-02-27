/*
 * QT_DAC.c
 *
 *  Created on: 16/01/2019
 *      Author: james
 */

#include "SpiLib/QT_SPI_SpiLib.h"

static uint16_t data;

/**
 * Converts voltage used internally by MCU into a uint16 value for the DAC to use.
 */
static uint16_t convertToDacVoltage(uint16_t voltage) {
    return voltage; // TODO
}

/**
 * This method sends a particular bit value to the DAC
 *
 * This method blocks until all other transmissions have finished.
 **/
void QT_DAC_setOutputValue(uint16_t value) {
    data = convertToDacVoltage(value);
    while(!QT_SPI_transmit((byte *) &data, 2, &DAC, NULL));
}
