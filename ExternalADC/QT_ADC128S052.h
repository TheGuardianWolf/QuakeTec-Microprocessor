#ifndef _QT_ADC128S052_H
#define _QT_ADC128S052_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#define NUMBER_OF_CHANNELS      (8)

#define ERROR_NONE              (0)
#define ERROR_SPI_BUS           (1)
#define ERROR_INVALID_CHANNEL   (2)

void ADC128S052_Init();

/*
 * Switches the ADC channel.
 * Returns an error code:
 *  0   : No error.
 *  1   : SPI bus error.
 *  2   : Invalid channel specified.
 */
uint8_t ADC128S052_SelectChannel(uint8_t u8_Channel);

/*
 * Switches the ADC channel.
 * Returns an error code:
 *  0   : No error.
 *  1   : SPI bus error.
 */
uint8_t ADC128S052_Read(uint16_t *pu16_Value);

#endif /* _QT_ADC128S052_H */
