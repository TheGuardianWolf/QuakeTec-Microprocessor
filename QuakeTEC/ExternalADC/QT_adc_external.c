/*
 * QT_adc_external.c
 *
 *  Created on: 24/02/2019
 *      Author: james
 */

#include "QT_adc_external.h"

#define ADC_SHIFT 1

static volatile uint16_t adcData;
static volatile bool doneReceiving;
static volatile bool doneTransmitting;


extern device_t ADC;

static byte sendData[4] = { 0 };

static void adcHandler(const byte *data) {
    adcData = (data[0]<<8) + data[1];
    uint16_t a = adcData;
    doneReceiving = true;
}

/**
 * This must be called after the SPI has initialised
 */
void QT_EADC_initialise() {
    QT_SPI_setReceiveHandler(adcHandler, 2, &ADC);
}

/**
 * Reads the ADC0 voltage.
 * This function returns false if the device was busy. If this is the case
 * no action is taken.
 **/

/**
 * Reads the ADC1 voltage.
 * This function returns false if the device was busy. If this is the case
 * no action is taken.
 **/

/**
 * Reads the ADC2 voltage.
 * This function returns false if the device was busy. If this is the case
 * no action is taken.
 **/

void adc_handler(const byte * data) {
    adcData = (data[0]<<8) + data[1];
    uint16_t a = adcData;
    doneReceiving = true;
}

float getAdcVoltage() {
    uint16_t result = getAdcValue();
    float voltage = (float) result * EADC_VOLTAGE / EADC_RESOLUTION;
    return voltage;
}

uint16_t getAdcValue() {
    while (!doneReceiving) {;}
    uint16_t data = adcData;
    return adcData;
}

void QT_adc_doneTransmitting(bool succeeded){
    doneTransmitting = true;
}

bool adcRead(AdcPin adcPin) {
    doneReceiving = false;
    doneTransmitting = false;
    uint16_t data = adcData;
    byte sendData[2] = {adcPin<<3 , 0x00};
    ADC.receiveHandler = adcHandler;
    QT_SPI_transmit(sendData, 2, &ADC, QT_adc_doneTransmitting);
    while (!doneTransmitting) {;}
    __delay_cycles(400);
//    while (!QT_SPI_isDataSent(&DAC)) {;}
    return true;

}


