/*
 * Includes
 */
#include "Sweep/QT_SW_sweep.h"

/*
 * Defines
 */

const float INITIAL_GAIN = 2.0;
const float DIGIPOT_MAX_GAIN = 1000.0;
const float DIGIPOT_MIN_GAIN = 2.0;
const float SWEEP_MIN_VOLTAGE = -15.0;
const float SWEEP_MAX_VOLTAGE = 15.0;
const float SWEEP_MIN_REPEATS = 10;
const float SWEEP_MAX_REPEATS = 10;
const float SWEEP_ADC_MID_VOL = (1.25 * EADC_RESOLUTION / EADC_VOLTAGE);

#define SWEEP_LENGTH 500.0 // sweep length in ms
#define SWEEP_MAX_NUM_SAMPLES 500
#define PRESWEEP_LENGTH 150.0
#define PRESWEEP_MAX_NUM_SAMPLES 20

#define SMOOTH_WINDOW_SIZE 5
#define NUM_SMOOTH_PASSES 10

#define SWEEP_DATA_NUM_2BYTES_PADDING 2

static volatile float maxSweepValue;
static volatile float minSweepValue;
static volatile float digipotGain;
static volatile float dacOffset;

extern volatile bool sweepFlag;

static uint16_t sweepData[SWEEP_MAX_NUM_SAMPLES] = {1000};
volatile uint16_t sweepDataLength = 0;
static volatile float testMultiplier = 0.01;

/*
 * Type definitions
 */
typedef bool(*adc_retrieve_func_t)(adc_read_func_t callback);

/*
 * Private variables
 */

/**
 * For retrieving voltages from ADC
 */
//static bool adcValueProcessed;
//volatile static uint16_t adcValue;

/*
 * Private functions
 */

/**
 * Create the default settings object.
 */
static void QT_SW_setDefaultState() {
    digipotGain = DIGIPOT_MIN_GAIN;
    //TODO set digipot to this gain
    QT_DAC_reset();


}

sweep_settings_t QT_SW_createSweepSettings(int numSweeps, float minSweepVoltage, float maxSweepVoltage, int numberOfSamples, float sweepTime) {
    sweep_settings_t settings;

//    dpGain = dpGain >  DIGIPOT_MAX_GAIN ? DIGIPOT_MAX_GAIN : dpGain;
//    dpGain = dpGain < DIGIPOT_MIN_GAIN ? DIGIPOT_MIN_GAIN : dpGain;

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

//bool QT_SW_setDacSweepValue(sweep_settings_t * settings, int iteration) {
//    float value;
//    value = 1024*(settings->minSweepVoltage +
//            ((float) iteration * (settings->maxSweepVoltage - settings->minSweepVoltage) / settings->numberOfSamples))/5;
//    uint16_t adcValue = value + settings->dacOffset;
//    if (adcValue > DAC_MAX_VALUE) {
//        adcValue = DAC_MAX_VALUE;
//    } else if (adcValue < DAC_MIN_VALUE) {
//        adcValue = DAC_MIN_VALUE;
//    }
//    //send adc packet
//    return true;
//}

static int QT_SW_sweep(float startSweepVoltage, float endSweepVoltage, int samples, float period, bool useAdc) {
    //code for testing adaptive gain starts
//    startSweepVoltage = (startSweepVoltage + endSweepVoltage)/2

    //stops
    volatile struct timer* sp_timer = QT_TIMER_startPeriodicTask(SAMPLE_PROBE, period*samples, period);
    float sweepVoltage = startSweepVoltage;
    float increment = (endSweepVoltage - startSweepVoltage) / ((float) samples);
    float maxRange = startSweepVoltage > endSweepVoltage ? startSweepVoltage : endSweepVoltage;
    float minRange = startSweepVoltage < endSweepVoltage ? startSweepVoltage : endSweepVoltage;
    int i = 0;
    int j =0;
    while ((sp_timer->command != TIMER_STOP) && (sweepVoltage <= maxRange) && (sweepVoltage >= minRange) && !exitCommand && (i < samples)) {
        if (sweepFlag) {
            QT_DAC_setVoltage(sweepVoltage);

            if (useAdc) {
                //Read sweeping probe
                adcRead(ADC0);
                sweepData[i] = getAdcValue();
//                i++;
                //Read floating probe
                adcRead(ADC7);
//                sweepData[i] = getAdcValue();
                i++;
            }

            sweepVoltage = sweepVoltage + increment;
            sweepFlag = false;
        }

    }
    QT_TIMER_stopPeriodicTask(sp_timer);

    return sweepVoltage;
}

void QT_SW_conductSweep(sweep_settings_t * settings) {
    float period = (settings->sweepTime)/(settings->numberOfSamples);
    sweepDataLength = settings->numberOfSamples;
    int i;

    //for gain testing
    float dif = 0.001 * digipotGain * 15;
    float minvol = -dif;
    float maxvol = dif;

    i = QT_SW_sweep(0, minvol, (int) settings->numberOfSamples / 2, period, false);

    i = QT_SW_sweep(minvol, maxvol, settings->numberOfSamples, period, true);

    i = QT_SW_sweep(maxvol, minvol, (int) settings->numberOfSamples / 2, period, false);

    //end gain testing

//    i = QT_SW_sweep(0, settings->minSweepVoltage, (int) settings->numberOfSamples / 2, period, false);
//
//    i = QT_SW_sweep(settings->minSweepVoltage, settings->maxSweepVoltage, settings->numberOfSamples, period, true);
//
//    i = QT_SW_sweep(i, 0, (int) settings->numberOfSamples / 2, period, false);

    float f;
    int a =1;
    int j = 0;
    for (j = 0; j < 1000; j++) {
        f = sweepData[j];
        a = j;
    }
    a++;
}


void QT_SW_getPlasmaData() {
    QT_SW_setDefaultState();
    int i;

    sweep_settings_t settings = QT_SW_presweep();



}

static sweep_settings_t QT_SW_presweep() {
    int i;
    float dgain;
    sweep_settings_t settings = QT_SW_createSweepSettings(1, SWEEP_MIN_VOLTAGE, SWEEP_MAX_VOLTAGE, PRESWEEP_MAX_NUM_SAMPLES, PRESWEEP_LENGTH);
    dgain = digipotGain;

    for (i = 0; i < 7; i++) {
        QT_SW_conductSweep(&settings);
        QT_SW_adaptiveGain();
        dgain = digipotGain;

    }


    return settings;
}

static float QT_SW_adaptiveGain() {
    float gain;
    float ifNegValue = 10000;
    uint16_t currentMax = QT_COM_max(sweepData, sweepDataLength);
    uint16_t currentMin = QT_COM_min(sweepData, sweepDataLength);

    float ionCurrrentGain = (SWEEP_ADC_MID_VOL - EADC_MIN_VALUE) / ( SWEEP_ADC_MID_VOL - (float) currentMin);
    float electronCurrrentGain = (EADC_MAX_VALUE - SWEEP_ADC_MID_VOL) / ((float) currentMax - SWEEP_ADC_MID_VOL);
    ionCurrrentGain = ionCurrrentGain <= 0 ? ifNegValue : ionCurrrentGain;
    electronCurrrentGain = electronCurrrentGain <= 0 ? ifNegValue : electronCurrrentGain;

    gain = ionCurrrentGain > electronCurrrentGain ? electronCurrrentGain : ionCurrrentGain;

    gain = gain > 10 ? 10 : gain;
    digipotGain = digipotGain * gain > DIGIPOT_MAX_GAIN ? DIGIPOT_MAX_GAIN : digipotGain * gain;
    digipotGain = digipotGain < DIGIPOT_MIN_GAIN ? DIGIPOT_MIN_GAIN : digipotGain;

    //TODO set new digipot gain
    return gain;
}

/**
 * Sets initial values of DigiPot and DAC.
 */
//static void initialiseDigiPotAndDac(sweep_settings_t settings) {
//
//    // Set gain on DigiPot
//    QT_DIGIPOT_setGain(settings.digiPotGain);
//
//    // Set starting voltage on DAC
//    QT_DAC_setOutputValue(settings.minSweepVoltage);
//}
//
//// TODO check exit flags before doing stuff
//
///**
// * Remembers value returned by ADC.
// */
//static void processAdcValue(uint16_t voltage) {
//    adcValue = voltage;
//    adcValueProcessed = true;
//}
//
///**
// * Asks ADC for a value according to the retrieve function and waits until value has been returned.
// */
//static uint16_t retrieveAdcValue(adc_retrieve_func_t retrieveFunction) {
//
//    adcValueProcessed = false;
//    bool deviceReady;
//
//    // Wait until SPI line is free to measure sweep voltage
//    do {
//        deviceReady = (*retrieveFunction)((adc_read_func_t) processAdcValue);
//    }
//    while (!deviceReady);
//
//    // Wait until sweep voltage handler is called
//    while (!adcValueProcessed);
//
//    uint16_t value = adcValue;
//    adcValueProcessed = false;
//
//    return value;
//}
//
///**
// * Sweeps from start DAC voltage to end DAC voltage in SWEEP_LENGTH ms. This function updates the DAC so that it
// * passes through every value.
// */
//static sweep_data_t sweep(sweep_settings_t settings, int numSamples) {
//
//    sweep_data_t sweepData;
//    uint16_t dacVoltage;
//    uint16_t sampleWidth = (settings.maxSweepVoltage - settings.maxSweepVoltage) * VOLTAGE_MULTIPLE / numSamples;
//    uint8_t sampleCounter = 0; // determines when to take readings from the ADC
//
//    sweepData.bufferLength = SWEEP_DATA_NUM_2BYTES_PADDING;
//
//    // Sweep up from start DAC value to end DAC value
//    for (dacVoltage = settings.minSweepVoltage; dacVoltage <= settings.maxSweepVoltage; dacVoltage++) {
//
//        QT_DAC_setOutputValue(dacVoltage);              // set DAC voltage
//        sampleCounter++;
//        // TODO delay, check PCB temp etc
//
//        if (sampleCounter == sampleWidth) {             // ready to sample
//            uint16_t sweepVoltage = retrieveAdcValue(&QT_EADC_measureSweepVoltage);
//            uint16_t refVoltage = retrieveAdcValue(&QT_EADC_measureFloatVoltage);
//            QT_FRAM_write(sweepVoltage - refVoltage + settings.dacOffset);   // add sweep voltage data
//            sweepData.bufferLength++;         // increment buffer length
//        }
//    }
//
//    // Sweep down from end DAC value to start DAC value TODO make this async
//    for (dacVoltage = settings.maxSweepVoltage; dacVoltage <= settings.minSweepVoltage; dacVoltage--) {
//        QT_DAC_setOutputValue(dacVoltage);              // set DAC voltage
//    }
//
//    return sweepData;
//}
//
///**
// * Sweeps between sweepSettings.minSweepVoltage and sweepSettings.maxSweepVoltage and uses the maximum LP voltage to
// * set digiPot resistance for the sweep.
// */
//static void setPreSweepDigiPotGain(sweep_settings_t *sweepSettingsPtr) {
//
//    // Set DigiPot to its required gain
//    QT_DIGIPOT_setGain(sweepSettingsPtr->digiPotGain);
//
//    // Conduct sweep and retrieve data
//    sweep_data_t preSweepData = sweep(*sweepSettingsPtr, NUM_SAMPLES_PRESWEEP);
//
//    // Find max sweep voltage
//    uint16_t localMaxSweepVoltage = QT_COM_max(preSweepData.sweepVoltages, preSweepData.bufferLength);
//
//    sweepSettingsPtr->digiPotGain = (float) MAX_SWEEP_VOLTAGE_TARGET / localMaxSweepVoltage;
//}
//
///**
// * Carries out a running smooth on presweep data.
// * TODO do we want a running smooth?
// */
//static void smoothPreSweepData(sweep_data_t *preSweepDataPtr) {
//    int pass;
//
//    // Smooth the data NUM_SMOOTH_PASSES number of times
//    for (pass = 0; pass < NUM_SMOOTH_PASSES; pass++) {
//
//        int i, j;
//
//        // TODO smooth endpoints
//
//        // Smooth middle region
//        for (i = SMOOTH_WINDOW_SIZE/2 - 1; i <= preSweepDataPtr->bufferLength - SMOOTH_WINDOW_SIZE/2; i++) {
//
//            uint32_t total = 0;
//
//            for (j = i + 1 - SMOOTH_WINDOW_SIZE/2; j < i - 1 + SMOOTH_WINDOW_SIZE/2; j++) {
//                total += preSweepDataPtr->sweepVoltages[j];
//            }
//
//            preSweepDataPtr->sweepVoltages[i] = (uint16_t)(total / SMOOTH_WINDOW_SIZE);
//        }
//    }
//}
//
///**
// * Finds the DAC voltages at the two inflection points in the sweep data.
// */
//static void findInflectionPoints(sweep_data_t *preSweepDataPtr, uint16_t *firstInflectionPtr, uint16_t *secondInflectionPtr) {
//    // TODO
//}
//
///**
// * Sets the minimum and maximum DAC values for the sweep. SetPreSweepDigiPotGain() should be called first.
// */
//static void setPreSweepDacLimits(sweep_settings_t *sweepSettingsPtr) {
//
//    // Set DigiPot to its required resistance
//    QT_DIGIPOT_setGain(sweepSettingsPtr->digiPotGain);
//
//    // Conduct sweep - will block until the sweep is completed
//    sweep_data_t sweepData = sweep(*sweepSettingsPtr, NUM_SAMPLES_PRESWEEP);
//
//    // Smooth data
//    smoothPreSweepData(&sweepData);
//
//    // Find DAC voltages at the inflection points
//    uint16_t firstInflection;
//    uint16_t secondInflection;
//    findInflectionPoints(&sweepData, &firstInflection, &secondInflection);
//
//    // Set DAC limits
//    uint16_t targetThirdOfDacRange = secondInflection - firstInflection;
//    sweepSettingsPtr->minSweepVoltage = firstInflection - targetThirdOfDacRange;
//    sweepSettingsPtr->maxSweepVoltage = secondInflection + targetThirdOfDacRange;
//}
//
///**
// * Puts the sweep into a safe state.
// */
//static void placeSweepInSafeState() {
//    // TODO
//}
//
///**
// * Uses feedback from ADC1_2 to find DAC offset. Adds this to settings.
// */
//static void adjustForDacOffset(sweep_settings_t *settings) {
//
//    uint16_t adcCurrent = retrieveAdcValue(&QT_EADC_measureSweepCurrent);
//    float spVoltageFromAdc = -(55 * adcCurrent)/409600 + 1.65; // TODO make these magic numbers #defines
//    float spVoltage = retrieveAdcValue(&QT_EADC_measureSweepVoltage) / VOLTAGE_MULTIPLE;
//    settings->dacOffset = spVoltageFromAdc - spVoltage;
//}
//
///*
// * Public functions
// */
//
///**
// * Calibrates DAC and initialises DigiPot resistance and DAC limits before main sweep.
// */
//sweep_settings_t QT_SW_conductPreSweep() {
//
//    sweep_settings_t settings = createDefaultSettings();
//
//    // Set up starting DigiPot and DAC values
//    initialiseDigiPotAndDac(settings);
//
//    // Find DAC offset
//    adjustForDacOffset(&settings);
//
//    // Set new DigiPot resistance
//    setPreSweepDigiPotGain(&settings);
//
//    // Resweep with new DigiPot resistance to find DAC limits
//    setPreSweepDacLimits(&settings);
//
//    return settings;
//}
//
///**
// * Conducts a sweep. When sweep data is available, conductSweep() tells the OBC by calling QT_LP_signalSweepDataReady().
// */
//sweep_data_t QT_SW_conductSweep(sweep_settings_t settings) {
//
//    // Set DigiPot to its required resistance
//    QT_DIGIPOT_setGain(settings.digiPotGain);
//
//    // Conduct sweep
//    sweep_data_t sweepData = sweep(settings, NUM_SAMPLES_SWEEP);
//
//    return sweepData;
//}


