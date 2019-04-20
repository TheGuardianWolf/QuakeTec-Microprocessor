/*
 * QT_DAC.c
 *
 *  Created on: 16/01/2019
 *      Author: james
 */

#include "QT_DAC.h"

#define DAC_RESOLUTION 4095.0
#define DAC_VOLTAGE 3.3
#define MAX_SWEEP_VOLTAGE 15.0
#define MIN_SWEEP_VOLTAGE -15.0
#define ZERO_DAC (DAC_VOLTAGE/2)
#define SWEEP_SCALE (1.0/(30.0/DAC_VOLTAGE))

static volatile bool doneTransmitting;

extern device_t DAC;


/**
 * Converts voltage used internally by MCU into a uint16 value for the DAC to use.
 */
static uint16_t convertToDacVoltage(float voltage) {
    voltage = voltage > DAC_VOLTAGE ? DAC_VOLTAGE : voltage;
    voltage = voltage < 0 ? 0 : voltage;
    uint16_t convertedValue = (DAC_RESOLUTION * voltage) / DAC_VOLTAGE;
    return convertedValue;
}


static uint16_t convertToDacSweepVoltage(float voltage) {
    voltage = voltage > MAX_SWEEP_VOLTAGE ? MAX_SWEEP_VOLTAGE : voltage;
    voltage = voltage < MIN_SWEEP_VOLTAGE ? MIN_SWEEP_VOLTAGE : voltage;
    uint16_t converted_value = (DAC_RESOLUTION / DAC_VOLTAGE) * (ZERO_DAC - (SWEEP_SCALE * voltage));
    return converted_value;
}
/**
 * This method sends a particular bit value to the DAC
 *
 * This method blocks until all other transmissions have finished.
 **/

void QT_DAC_doneTransmitting(bool succeeded){
    doneTransmitting = true;
}


void QT_DAC_setVoltage(float voltage) {
    doneTransmitting = false;
    uint16_t convertedValue = convertToDacSweepVoltage(voltage);
    byte b1 = (convertedValue & 0xFF00)>>8;
    byte b2 = convertedValue & 0x00FF;
    byte sendData[2] = { b1, b2};
    QT_SPI_transmit(sendData, 2, &DAC, QT_DAC_doneTransmitting);
    while (!doneTransmitting) {;}
    __delay_cycles(400);
//    while (!QT_SPI_isDataSent(&DAC)) {;}

}

void QT_DAC_reset() {
    QT_DAC_setVoltage(0);
}
