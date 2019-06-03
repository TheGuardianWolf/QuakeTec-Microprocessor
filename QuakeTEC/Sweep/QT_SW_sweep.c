#include "Sweep/QT_SW_sweep.h"

extern volatile bool sweepFlag;

#define SWEEP_LENGTH 300 // sweep length in ms
#define PRESWEEP_LENGTH 300.0
#define PRESWEEP_MAX_NUM_SAMPLES 50

#define SWEEP_DATA_NUM_2BYTES_PADDING 2

const float INITIAL_GAIN = 2.0;
const float DIGIPOT_MAX_GAIN = 1000.0;
const float DIGIPOT_MIN_GAIN = 2.0;
const float SWEEP_MIN_VOLTAGE = -15.0;
const float SWEEP_MAX_VOLTAGE = 15.0;
const float SWEEP_MIN_REPEATS = 10;
const float SWEEP_MAX_REPEATS = 10;
const float SWEEP_ADC_MID_VOL = (1.67 * EADC_RESOLUTION / EADC_VOLTAGE);

static volatile float maxSweepValue;
static volatile float minSweepValue;
static volatile float digipotGain;
static volatile float dacOffset;
static volatile float testMultiplier = 0.01;
static volatile uint16_t sweepDataLength = 0;
static byte fpVoltage[2];

static byte sweepData[2 * SWEEP_MAX_NUM_SAMPLES * SWEEP_REPETITIONS] = {0};
static volatile int dataPointer = 0;

static void QT_SW_setDefaultState() {
    digipotGain = DIGIPOT_MIN_GAIN;
    QT_DIGIPOT_setGain(digipotGain);
    QT_DAC_reset();
    dataPointer = 0;
}

static int QT_SW_sweep(float startSweepVoltage, float endSweepVoltage, int samples, float period, bool useAdc) {
    volatile struct timer* sp_timer = QT_TIMER_startPeriodicTask(SAMPLE_PROBE, period*samples, period);
    float sweepVoltage = startSweepVoltage;
    float increment = (endSweepVoltage - startSweepVoltage) / ((float) samples);
    float maxRange = startSweepVoltage > endSweepVoltage ? startSweepVoltage : endSweepVoltage;
    float minRange = startSweepVoltage < endSweepVoltage ? startSweepVoltage : endSweepVoltage;
    uint16_t spValue;
    int asd = dataPointer;

    bool status;
    while ((sp_timer->command != TIMER_STOP) && (sweepVoltage <= maxRange) && (sweepVoltage >= minRange) &&
            (dataPointer <= (2 * SWEEP_MAX_NUM_SAMPLES * SWEEP_REPETITIONS)) && !exitCommand) {
//    for (i = 0; i<samples; i++) {
        if (sweepFlag) {
            status = QT_DAC_setVoltage(sweepVoltage);
            if (status == 0) {
                ERROR_STATUS |= (BIT8|BIT9);
                queueEvent(PL_EVENT_ERROR);
            }

            if (useAdc) {
                //Read sweeping probe
                spValue = QT_EADC_getAdcValue(ADC0);
                sweepData[dataPointer] = spValue >> 8;
                dataPointer++;
                sweepData[dataPointer] = spValue & 0x00FF;
                dataPointer++;
                asd = dataPointer;
            }

            sweepVoltage = sweepVoltage + increment;
            sweepFlag = false;
        }

    }
    QT_TIMER_stopPeriodicTask(sp_timer);

    return sweepVoltage;
}

static void QT_SW_conductSweep(sweep_settings_t * settings) {
    float period = (settings->sweepTime)/(settings->numberOfSamples);
    sweepDataLength = settings->numberOfSamples;
    int vol, i;

    for (i = 0; i < SWEEP_REPETITIONS; i++) {
        vol = QT_SW_sweep(0, settings->minSweepVoltage, (int) settings->numberOfSamples / 2, period, false);

        vol = QT_SW_sweep(settings->minSweepVoltage, settings->maxSweepVoltage, settings->numberOfSamples, period, true);

        vol = QT_SW_sweep(vol, 0, (int) settings->numberOfSamples / 2, period, false);
    }

    float f;
    int a =1;
    int j = 0;
    for (j = 0; j <  2 * SWEEP_MAX_NUM_SAMPLES * SWEEP_REPETITIONS; j++) {
        f = sweepData[j];
        a = j;
    }
    a++;
}

static float QT_SW_adaptiveGain() {
    float gain;
    float ifNegValue = 10000;

    uint16_t currentMax = QT_COM_maxByteArray(sweepData, 2 * SWEEP_MAX_NUM_SAMPLES * SWEEP_REPETITIONS);
    uint16_t currentMin = QT_COM_minByteArray(sweepData, 2 * SWEEP_MAX_NUM_SAMPLES * SWEEP_REPETITIONS);

    float ionCurrrentGain = (SWEEP_ADC_MID_VOL - EADC_MIN_VALUE) / ( SWEEP_ADC_MID_VOL - (float) currentMin);
    float electronCurrrentGain = (EADC_MAX_VALUE - SWEEP_ADC_MID_VOL) / ((float) currentMax - SWEEP_ADC_MID_VOL);
    ionCurrrentGain = ionCurrrentGain <= 0 ? ifNegValue : ionCurrrentGain;
    electronCurrrentGain = electronCurrrentGain <= 0 ? ifNegValue : electronCurrrentGain;

    gain = ionCurrrentGain > electronCurrrentGain ? electronCurrrentGain : ionCurrrentGain;

    gain = gain > 10 ? 10 : gain;
    digipotGain = digipotGain * gain > DIGIPOT_MAX_GAIN ? DIGIPOT_MAX_GAIN : digipotGain * gain;
    digipotGain = digipotGain < DIGIPOT_MIN_GAIN ? DIGIPOT_MIN_GAIN : digipotGain;

    QT_DIGIPOT_setGain(digipotGain);

    return gain;
}

static sweep_settings_t QT_SW_presweep() {
    int i;
    float dgain, gainMultiplier;
    sweep_settings_t settings = QT_SW_createSweepSettings(1, SWEEP_MIN_VOLTAGE, SWEEP_MAX_VOLTAGE, PRESWEEP_MAX_NUM_SAMPLES, PRESWEEP_LENGTH);
    dgain = digipotGain;

    for (i = 0; i < 7; i++) {
        QT_SW_conductSweep(&settings);
        if (exitCommand) {
            break;
        }
        gainMultiplier = QT_SW_adaptiveGain();
        dataPointer = 0;
        dgain = digipotGain;
    }
    settings = QT_SW_createSweepSettings(1, SWEEP_MIN_VOLTAGE, SWEEP_MAX_VOLTAGE, SWEEP_MAX_NUM_SAMPLES, SWEEP_LENGTH);
    return settings;
}

sweep_settings_t QT_SW_createSweepSettings(int numSweeps, float minSweepVoltage, float maxSweepVoltage, int numberOfSamples, float sweepTime) {
    sweep_settings_t settings;

    numSweeps = numSweeps > SWEEP_MAX_REPEATS ? SWEEP_MAX_REPEATS : numSweeps;
    numSweeps = numSweeps < SWEEP_MAX_REPEATS ? SWEEP_MAX_REPEATS : numSweeps;

    maxSweepVoltage = maxSweepVoltage > SWEEP_MAX_VOLTAGE ? SWEEP_MAX_VOLTAGE : maxSweepVoltage;
    minSweepVoltage = minSweepVoltage < SWEEP_MIN_VOLTAGE ? SWEEP_MIN_VOLTAGE : minSweepVoltage;
    int dacOffset = 0;

    numberOfSamples = numberOfSamples > SWEEP_MAX_NUM_SAMPLES ? SWEEP_MAX_NUM_SAMPLES: numberOfSamples;

//    settings.digiPotGain = dpGain;
    settings.numberOfSweeps = numSweeps;
    settings.maxSweepVoltage = maxSweepVoltage;
    settings.minSweepVoltage = minSweepVoltage;
    settings.dacOffset = dacOffset;
    settings.numberOfSamples = numberOfSamples;
    settings.sweepTime = sweepTime;
    return settings;
}

byte* QT_SW_getFloatingProbeVoltage() {
    uint16_t probeVol;
    uint16_t multiplier = 1024;
    probeVol = QT_EADC_getAdcVoltage(ADC1);
    probeVol = multiplier * probeVol;
    fpVoltage[0] = probeVol >> 8;
    fpVoltage[1] = probeVol & 0x00FF;
    return fpVoltage;
}

byte* QT_SW_getSweepData() {
    return sweepData;
}

void QT_SW_getPlasmaData() {
    QT_SW_setDefaultState();
    QT_PWR_turnOnGuard();

    sweep_settings_t settings = QT_SW_presweep();
//    sweep_settings_t settings = QT_SW_createSweepSettings(1, SWEEP_MIN_VOLTAGE, SWEEP_MAX_VOLTAGE, SWEEP_MAX_NUM_SAMPLES, SWEEP_LENGTH);


    __delay_cycles(1000000);
    QT_SW_conductSweep(&settings);

    QT_PWR_turnOffGuard();
    QT_SW_setDefaultState();

}

