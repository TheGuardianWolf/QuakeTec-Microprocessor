#ifndef _QT_AD5292_H
#define _QT_AD5292_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

void AD5292_Init();

/*
 * Use the software shutdown command to put the device in normal operating mode.
 * Returns an error code:
 * 	0	: No error.
 * 	1	: SPI bus error.
 */
uint8_t AD5292_SwitchOn();

/*
 * Use the software shutdown command to put the device in shutdown mode.
 * Returns an error code:
 * 	0	: No error.
 * 	1	: SPI bus error.
 */
uint8_t AD5292_SwitchOff();

/*
 * Sets the RDac value from 0-1023.
 * Returns an error code:
 * 	0	: No error.
 * 	1	: SPI bus error.
 * 	2	: Invalid input value.
 */
uint8_t AD5292_Set(uint16_t u16_Value);

/*
 * Gets the RDac value.
 * Returns an error code:
 * 	0	: No error.
 * 	1	: SPI bus error.
 */
uint8_t AD5292_Get(uint16_t *u16_Value);

/*
 * Stores the current wiper setting permanently to the 20-TP memory.
 * Returns an error code:
 * 	0	: No error.
 * 	1	: SPI bus error.
 */
uint8_t AD5292_Store();

/*
 * Resets the wiper value to the value stored in the 20-TP memory.
 * Returns an error code:
 * 	0	: No error.
 * 	1	: SPI bus error.
 */
uint8_t AD5292_Reset();



#endif /* _QT_AD5292_H */
