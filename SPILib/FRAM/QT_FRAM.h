/*
 * QT_FRAM.h
 *
 *  Created on: 25/02/2019
 *      Author: james
 */

#ifndef FRAM_QT_FRAM_H_
#define FRAM_QT_FRAM_H_

#include "driverlib.h"

void QT_FRAM_initialise();

/** Writes a single value to FRAM at writehead and increments the writehead. */
void QT_FRAM_write(uint16_t data);

/** Readys the FRAM buffer for writing. */
void QT_FRAM_reset();

/** Returns a pointer to the data. */
uint16_t *QT_FRAM_dataPtr();

/** Gets the distance between the readhead and the writehead. */
int QT_FRAM_length();


#endif /* FRAM_QT_FRAM_H_ */
