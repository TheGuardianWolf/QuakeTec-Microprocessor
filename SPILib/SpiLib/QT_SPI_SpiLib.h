#ifndef SPILIB_H
#define SPILIB_H

/*
 * Includes
 */
#include "Common/QT_COM_common.h"

/*
 * Type definitions
 */
typedef void(*receive_handler_func)(const byte * data);
typedef void(*transmit_handler_func)(bool succeeded);

typedef struct {
    bool isSlave;
    uint16_t spiBaseAddress;
    receive_handler_func receiveHandler;
    byte expectedLength;

    // CS storage
    uint8_t csPort, csPin;
} device_t;

/**
 * Variable declarations
 */
extern device_t OBC;
extern device_t ADC;
extern device_t DAC;
extern device_t DIGIPOT;

/*
 * Public functions
 */

/**
 * Sets up the SPI library
 */
void QT_SPI_initialise();

/**
 * Returns immediately. Returns true if the system was ready to send data. If it returns false the data was not sent.
 * The callback will be called by the interrupt handler when the transmission has finished. If the callback is NULL nothing occurs
 */
bool QT_SPI_transmit(const byte *dataPtr, uint16_t length, device_t *device, transmit_handler_func callback);

/**
 * Registers a function that handles incoming data. This will be called when length bytes have been recevied.
 */
void QT_SPI_setReceiveHandler(receive_handler_func handler, byte length, device_t *device);

/**
 * Starts clocking data in from the slave and sets the CS line. This will cause the handler to be called.
 */
void QT_SPI_listenToSlave(device_t *device);

/**
 * Stops listening to the current slave.
 */
void QT_SPI_stopListeningToSlave();

/**
 * Returns true if all data has been sent.
 */
bool QT_SPI_isDataSent();

#endif
