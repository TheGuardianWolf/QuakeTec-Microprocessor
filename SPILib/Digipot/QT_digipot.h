/*
 * QT_digipot.h
 *
 *  Created on: 16/01/2019
 *      Author: james
 */

#ifndef QT_DIGIPOT_H_
#define QT_DIGIPOT_H_

/**
 * This sets the control bits for the Digipot. If data is not ready to send this
 * method will block until the data starts to send.
 */
void QT_DIGIPOT_setControlBits();

/**
 * This sets the data bits for the Digipot. If data is not ready to send this
 * method will block until the data starts to send.
 */
void QT_DIGIPOT_setDataBits();

#endif /* QT_DIGIPOT_H_ */
