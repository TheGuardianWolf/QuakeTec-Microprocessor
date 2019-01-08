#ifndef SPILIB_H
#define SPILIB_H

/*
 * Includes
 */
#include <stdbool.h>
#include <stdint.h>

/*
 * Type definitions
 */

typedef unsigned char byte;

typedef void(*handler_func)(const byte *);

typedef struct {
    bool isSlave;
    uint16_t spiBaseAddress;
    handler_func receiveHandler;
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
void initialise();

/**
 * Returns immediately. Returns true if the system was ready to send data. If it returns false the data was not sent.
 */
bool transmit(const byte *dataPtr, uint16_t length, device_t *device);

/**
 * Registers a function that handles incoming data. This will be called when length bytes have been recevied.
 */
void setReceiveHandler(handler_func handler, byte length, device_t *device);

/**
 * Starts clocking data in from the slave and sets the CS line. This will cause the handler to be called.
 */
void listenToSlave(device_t *device);

/**
 * Stops listening to the current slave.
 */
void stopListeningToSlave();

/**
 * Returns true if all data has been sent.
 */
bool isDataSent();

#endif
