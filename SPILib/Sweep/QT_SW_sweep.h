#ifndef QT_SW_SWEEP_H_
#define QT_SW_SWEEP_H_

/*
 * Includes
 */
#include <float.h>

/*
 * Type definitions
 */

typedef struct {
    float digiPotGain;
    uint16_t minDacVoltage;
    uint16_t maxDacVoltage;
} sweep_settings_t;

typedef struct {
    /** A pointer to the array of sweep voltages. This will be in FRAM, and should therefore not be written to. */
    uint16_t *sweepVoltages;

    /** The number of floats in the array. */
    int bufferLength;
} sweep_data_t;

/*
 * Public functions
 */

/**
 * Initialises DigiPot and DAC before main sweep, returning the settings object that represents.
 */
sweep_settings_t QT_SW_conductPreSweep();

/**
 * Conducts a sweep. When sweep data is available, conductSweep() tells the OBC by calling QT_LP_signalSweepDataReady().
 */
sweep_t QT_SW_conductSweep(sweep_settings_t settings);

#endif /* QT_SW_SWEEP_H_ */
