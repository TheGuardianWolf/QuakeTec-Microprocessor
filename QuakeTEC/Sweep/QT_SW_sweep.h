#ifndef QT_SW_SWEEP_H_
#define QT_SW_SWEEP_H_

/*
 * Includes
 */
#include <ExternalADC/QT_EADC.h>
#include "Common/QT_COM_common.h"
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

//static int QT_SW_sweep(float startSweepVoltage, float endSweepVoltage, int samples, float period, bool useAdc);
//static float QT_SW_adaptiveGain();
//static sweep_settings_t QT_SW_presweep();
//static void QT_SW_conductSweep(sweep_settings_t * settings);

void QT_SW_getPlasmaData();
sweep_settings_t QT_SW_createSweepSettings(int numSweeps, float maxSweepVoltage, float minSweepVoltage, int numberOfSamples, float sweepTime);


#endif /* QT_SW_SWEEP_H_ */
