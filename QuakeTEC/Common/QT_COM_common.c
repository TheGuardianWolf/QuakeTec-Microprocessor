/*
 * Includes
 */
#include "QT_COM_common.h"

/*
 * Public functions
 */

/**
 * Finds the maximum value in an array.
 */
uint16_t QT_COM_max(uint16_t *values, uint16_t length) {
    int i;
    uint16_t currentMax = values[0];

    for (i = 1; i < length; i++) {
        if (values[i] > currentMax) {
            currentMax = values[i];
        }
    }

    return currentMax;
}

/**
 * Finds the minimum value in an array.
 */
uint16_t QT_COM_min(uint16_t *values, uint16_t length) {
    int i;
    uint16_t currentMin = values[0];

    for (i = 1; i < length; i++) {
        if (values[i] < currentMin) {
            currentMin = values[i];
        }
    }

    return currentMin;
}
