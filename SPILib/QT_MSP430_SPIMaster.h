#ifndef _QT_MSP430_SPIMASTER_H
#define _QT_MSP430_SPIMASTER_H

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

/*
 * Macro to convert a frequency into the nearest clock divider value.
 */
#define SPIMASTER_SMCLK	4*1000000   // 4MHz
#define SPIMASTER_FREQUENCY_TO_DIVIDER(_freqencyHz) SPIMASTER_SMCLK / _freqencyHz

/*
 * Struct defining the configuration of the SPI device for a given slave.
 */
typedef struct
{
	bool b_ClockPolarity;
	bool b_ClockPhase;
	bool b_MSBFirst;
	uint8_t u8_CSPort;
	uint8_t u8_CSPin;
	uint16_t u16_ClockDivider;
	uint16_t u16_CSDelay_us;            // The delay between a rising or falling edge of CS and the start/end of the first/last clock cycle.
	uint16_t u16_TransmissionDelay_us;  // The delay after a transmission is complete before the
}
SPIMaster_Config_t;

/*
 * Initialise eUSCI_B as an SPI Master.
 */
void SPIMaster_Init();

/*
 * Configures and reserves the bus for a given slave device.
 * Must be called before attempting to transfer any data.
 * Returns an error code:
 * 	0	: No error.
 * 	1	: Bus has been reserved by this configuration.
 * 	2	: Bus has been reserved by a different configuration.
 */
uint8_t SPIMaster_StartTransfer(SPIMaster_Config_t *pt_Config);

/*
 * Transfers 8 bits between master and slave.
 * u8_Data should contain the data to transfer to the slave.
 * The value of u8_Data is modified to contain the data returned from the slave.
 * The chip select pin stored in the configuration input to SPIMaster_StartTransfer
 * will be pulled low during the transfer, and then high when the transfer is completed.
 * Returns an error code:
 * 	0	: No error.
 * 	1	: Bus has not been reserved by a call to SPIMaster_StartTransfer.
 */
uint8_t SPIMaster_Transfer8(uint8_t *u8_Data);

/*
 * Transfers 16 bits between master and slave.
 * Behaves similarly to SPIMaster_Transfer8.
 * Each byte will be transferred in separate transactions, however the chip select pin
 * will be held low for BOTH bytes.
 * Returns an error code:
 * 	0	: No error.
 * 	1	: Bus has not been reserved by a call to SPIMaster_StartTransfer.
 */
uint8_t SPIMaster_Transfer16(uint16_t *u16_Data);

/*
 * Releases the reservation put on the bus by SPIMaster_StartTransfer.
 * Must be called when data transfer has been completed.
 * Must be passed the configuration of the device which reserved the bus.
 * Returns an error code:
 * 	0	: No error.
 * 	1	: Bus was not reserved.
 * 	2	: pt_Config input does not match the configuration which was used to reserve the bus.
 */
uint8_t SPIMaster_EndTransfer(SPIMaster_Config_t *pt_Config);

/*
 * Equivalent to SPIMaster_EndTransfer, however will force the bus to be released.
 * This will not necessarily cause the code which reserved the bus to stop transmitting,
 * so should be used with caution.
 */
void SPIMaster_ForceEndTransfer();

#endif /* _QT_MSP430_SPIMASTER_H */
