/*
 * QT_DAC.h
 *
 *  Created on: 16/01/2019
 *      Author: james
 */

#ifndef QT_DAC_H_
#define QT_DAC_H_

#include "SpiLib/QT_SPI_SpiLib.h"
/**
 * This method sends a particular bit value to the DAC
 **/
void QT_DAC_setVoltage(float value);
void QT_DAC_reset();

#endif /* QT_DAC_H_ */
