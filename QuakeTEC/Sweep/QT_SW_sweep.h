#ifndef QT_SW_SWEEP_H_
#define QT_SW_SWEEP_H_

/*
 * Includes
 */
#include <stdlib.h>
#include <stdbool.h>
#include "Common/QT_COM_common.h"
#include "OBCInterface/QT_OBC_Interface.h"
#include "Digipot/QT_DIGIPOT.h"
#include "ExternalADC/QT_EADC.h"
#include "DAC/QT_DAC.h"
#include "Timer/QT_timer.h"
#include "PowerControl/QT_PWR_power.h"

#define SWEEP_MAX_NUM_SAMPLES 50
#define SWEEP_REPETITIONS 2
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

//static int QT_SW_sweep(float startSweepVoltage, float endSweepVoltage, int samples, float period, bool useAdc);
//static float QT_SW_adaptiveGain();
//static sweep_settings_t QT_SW_presweep();
//static void QT_SW_conductSweep(sweep_settings_t * settings);

void QT_SW_getPlasmaData();

sweep_settings_t QT_SW_createSweepSettings(int numSweeps, float maxSweepVoltage, float minSweepVoltage, int numberOfSamples, float sweepTime);

byte* QT_SW_getSweepData();

byte* QT_SW_getFloatingProbeVoltage();

#endif /* QT_SW_SWEEP_H_ */
