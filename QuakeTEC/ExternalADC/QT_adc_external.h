#include <stdbool.h>

/*
 * QT_adc_external.h
 *
 *  Created on: 24/02/2019
 *      Author: james
 */

#ifndef EXTERNALADC_QT_ADC_EXTERNAL_H_
#define EXTERNALADC_QT_ADC_EXTERNAL_H_

typedef void(*adc_read_func_t)(float data);

void QT_EADC_initialise();

/**
 * Reads the sweep current in A from the sweeping probe.
 * This function returns false if the device was busy. If this is the case
 * no action is taken.
 **/
bool QT_EADC_measureSweepCurrent(adc_read_func_t callback);

/**
 * Reads the voltage in V from the floating probe.
 * This function returns false if the device was busy. If this is the case
 * no action is taken.
 **/
bool QT_EADC_measureFloatVoltage(adc_read_func_t callback);

/**
 * Reads the output from the DAC in V.
 * This function returns false if the device was busy. If this is the case
 * no action is taken.
 **/
bool QT_EADC_measureSweepVoltage(adc_read_func_t callback);

#endif /* EXTERNALADC_QT_ADC_EXTERNAL_H_ */
