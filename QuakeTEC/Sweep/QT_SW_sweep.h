#ifndef QT_SW_SWEEP_H_
#define QT_SW_SWEEP_H_

/*
 * Includes
 */
#include "Common/QT_COM_common.h"
#include "ExternalADC/QT_adc_external.h"
#include "DAC/QT_DAC.h"
#include "Digipot/QT_digipot.h"
#include "FRAM/QT_FRAM.h"
#include "Timer/QT_timer.h"
#include "QT_LPMain.h"
#include <stdlib.h>

/*
 * Type definitions
 */

typedef struct {
//    float digiPotGain;
    int numberOfSweeps;
    float minSweepVoltage;
    float maxSweepVoltage;
    int dacOffset;
    int numberOfSamples;
    float sweepTime;
} sweep_settings_t;

typedef struct {
    /** A pointer to the array of sweep voltages. This will be in FRAM, and should therefore not be written to. */
    uint16_t *sweepVoltages;

    /** The number of floats in the array. */
    uint16_t bufferLength;
} sweep_data_t;

static int QT_SW_sweep(float startSweepVoltage, float endSweepVoltage, int samples, float period, bool useAdc);
static float QT_SW_adaptiveGain();

static sweep_settings_t QT_SW_presweep();
void QT_SW_getPlasmaData();

void QT_SW_conductSweep(sweep_settings_t * settings);

sweep_settings_t QT_SW_createSweepSettings(int numSweeps, float maxSweepVoltage, float minSweepVoltage, int numberOfSamples, float sweepTime);

//sweep_settings_t QT_SW_conductPreSweep();

/**
 * Conducts a sweep. When sweep data is available, conductSweep() tells the OBC by calling QT_LP_signalSweepDataReady().
 */
//sweep_data_t QT_SW_conductSweep(sweep_settings_t settings);
//void QT_SW_conductSweep(sweep_settings_t * settings);

#endif /* QT_SW_SWEEP_H_ */
