/*
 * QT_adc_external.h
 *
 *  Created on: 24/02/2019
 *      Author: james
 */

#ifndef EXTERNALADC_QT_ADC_EXTERNAL_H_
#define EXTERNALADC_QT_ADC_EXTERNAL_H_

#include <stdbool.h>
#include "SpiLib/QT_SPI_SpiLib.h"

#define EADC_RESOLUTION 4095.0
#define EADC_VOLTAGE 3.3
#define EADC_MIN_VALUE 150.0
#define EADC_MAX_VALUE 3950.0

typedef enum {
    ADC0,
    ADC1,
    ADC2,
    ADC3,
    ADC4,
    ADC5,
    ADC6,
    ADC7,
} AdcPin;



typedef void(*adc_read_func_t)(float data);
bool adcRead(AdcPin adcPin);

void QT_EADC_initialise();

uint16_t getAdcValue();

/**
 * Reads the sweep current in A from the sweeping probe.
 * This function returns false if the device was busy. If this is the case
 * no action is taken.
 **/

/**
 * Reads the output from the DAC in V.
 * This function returns false if the device was busy. If this is the case
 * no action is taken.
 **/
float getAdcVoltage();

#endif /* EXTERNALADC_QT_ADC_EXTERNAL_H_ */
