#ifndef SPILIB_H
#define SPILIB_H

#define OBCSPI_FILL_BYTE (0xFF)
#define OBCSPI_NULL_BYTE (0x00)

/*
 * Includes
 */
#include "Common/QT_COM_common.h"

typedef void (*QT_OBCSPI_ISRRxHandler_t)(uint8_t* const, const uint16_t);

typedef void (*QT_OBCSPI_ISRTxHandler_t)(uint8_t* const, const uint16_t, const uint16_t);

void QT_OBCSPI_init();

void QT_OBCSPI_disableInterrupts();

void QT_OBCSPI_enableInterrupts();

void QT_OBCSPI_clearRx();

void QT_OBCSPI_clearTx();

void QT_OBCSPI_writeTx(uint8_t* data, uint16_t dataLength);

void QT_OBCSPI_registerISRHandlers(QT_OBCSPI_ISRRxHandler_t handler1, QT_OBCSPI_ISRTxHandler_t handler2);

#endif
