//#ifndef _QT_MSP430_SPISLAVE_H
//#define _QT_MSP430_SPISLAVE_H
//
//#include <stdbool.h>
//#include <stdint.h>
//#include <stddef.h>
//
///*
// * Initialise eUSCI_A as an SPI Slave.
// */
//void SPISlave_Init();
//
///*
// * Disables interrupts related to the SPI slave.
// */
//void SPISlave_DisableInterrupts();
//
///*
// * Re-enables interrupts related to the SPI slave.
// */
//void SPISlave_EnableInterrupts();
//
///*
// * Queues a byte to be sent to the master.
// * Returns an error code:
// *  0   : No error.
// *  1   : Tx buffer is full.
// */
//uint8_t SPISlave_SendByte(uint8_t u8_ByteToSend);
//
///*
// * An example of how to retrieve the next byte from the Rx ring buffer.
// * Read the implementation of this function to see how to read from the ring
// * buffers correctly. The SPISlave_SendByte function shows a similar example
// * for writing to the buffers, though you'll probably never need to do this.
// * Returns an error code:
// *  0   : No error.
// *  1   : There was no byte to return.
// */
//uint8_t SPISlave_Example_GetNextRxByte(uint8_t *pu8_Byte);
//
//#endif /* _QT_MSP430_SPISLAVE_H */
