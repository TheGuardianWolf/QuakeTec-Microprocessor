/*
 * QT_pcbtemp.h
 *
 *  Created on: 30/01/2019
 *      Author: james
 */

#ifndef QT_PCBTEMP_H_
#define QT_PCBTEMP_H_

/**
 * Initialise the temperature
 */
void QT_TEMP_initialise();

/**
 * This function reads the temperature of the PCB, this function may block if the temperature reading is not ready yet.
 */
float QT_TEMP_readTemperature();

#endif /* QT_PCBTEMP_H_ */
