#ifndef QT_SW_SWEEP_H_
#define QT_SW_SWEEP_H_

/*
 * Includes
 */
#include <float.h>
#include <stdbool.h>
#include "driverlib.h"

/*
 * Defines
 */
#define SWEEP_DATA_BUFFER_SIZE 1024 // TODO how much memory can we spare?

/*
 * Type definitions
 */
typedef struct {
    uint16_t *sweepVoltages [SWEEP_DATA_BUFFER_SIZE];
    uint16_t bufferLength;
} sweep_data_t;

/*
 * Public functions
 */

/**
 * Initialises DigiPot and DAC before main sweep, calibrates DAC and collects data.
 */
void QT_SW_conductPreSweep();

/**
 * Puts the pre-sweep into a safe state and stops.
 */
void QT_SW_stopPreSweep();

/**
 * Conducts a sweep. When sweep data is available, conductSweep() tells the OBC by calling QT_LP_signalSweepDataReady().
 */
void QT_SW_conductSweep();

/**
 * Returns a pointer to a buffer of sweep data.
 */
sweep_data_t* QT_SW_retrieveSweepData();

/**
 * Puts the sweep into a safe state and stops.
 */
void QT_SW_stopSweep();

#endif /* QT_SW_SWEEP_H_ */
