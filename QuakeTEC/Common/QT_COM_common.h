#ifndef COMMON_QT_COMMON_C_
#define COMMON_QT_COMMON_C_

/*
 * Includes
 */
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <float.h>

#include "driverlib.h"

/*
 * Defines
 */
#define QT_COM_CLK_FREQUENCY 1000000 // TODO increase

#define NUM_VALUES_12_BITS 4096
#define NUM_VALUES_10_BITS 1024
#define MAX_VALUE_16_BITS 65535

/*
 * Type definitions
 */
typedef unsigned char byte;

/*
 * Public functions
 */

/**
 * Finds the maximum value in an array.
 */
uint16_t QT_COM_max(uint16_t *values, uint16_t length);

/**
 * Finds the minimum value in an array.
 */
uint16_t QT_COM_min(uint16_t *values, uint16_t length);

#endif /* COMMON_QT_COMMON_C_ */
