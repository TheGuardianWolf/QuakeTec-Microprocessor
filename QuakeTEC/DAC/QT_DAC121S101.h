#ifndef _QT_DAC121S101_H
#define _QT_DAC121S101_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#define ERROR_NONE              (0)
#define ERROR_SPI_BUS           (1)
#define ERROR_INPUT_TOO_HIGH    (2)
#define ERROR_MODE_INVALID      (3)

void DAC121S101_Init();

/*
 * Sets the value of the DAC.
 * Returns an error code:
 *  0   : No error.
 *  1   : SPI bus error.
 *  2   : Input value too high.
 */
uint8_t DAC121S101_Set(uint16_t u16_Value);

/*
 * Sets the power-down mode of the DAC.
 *
 * Mode details:
 *  0   : Normal operation.
 *  1   : Power-Down with 1k t GND.
 *  2   : Power-Down with 100k to GND.
 *  3   : Power-Down with Hi-Z.
 *
 * Returns an error code:
 *  0   : No error.
 *  1   : SPI bus error.
 *  2   : Mode invalid.
 */
uint8_t DAC121S101_SetMode(uint8_t u8_Mode);

#endif /* _QT_DAC121S101_H */
