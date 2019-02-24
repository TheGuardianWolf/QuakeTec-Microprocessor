/*
 * QT_pcbtemp.h
 *
 *  Created on: 30/01/2019
 *      Author: james
 */

#ifndef QT_ADC_H_
#define QT_ADC_H_

/**
 * Initialise the MC ADCs
 */
void QT_IADC_initialise();

/**
 * This function reads the temperature of the PCB, this function may block if the temperature reading is not ready yet.
 */
float QT_IADC_readTemperature();

/**
 * This function reads the battery current in A, this function will block for a small amount of time while the current is read (16 cycles)
 */
float QT_IADC_readCurrent();

#endif /* QT_ADC_H_ */
