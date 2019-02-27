/*
 * Includes
 */
#include "Sweep/QT_SW_sweep.h"

/*
 * Defines
 */

#define INITIAL_GAIN 2.0

#define DEFAULT_DAC_OFFSET 0
#define VOLTAGE_MULTIPLE 1000
#define DAC_MIN_VOLTAGE (0 * VOLTAGE_MULTIPLE)
#define DAC_MAX_VOLTAGE (5 * VOLTAGE_MULTIPLE)
#define MIN_SWEEP_VOLTAGE_TARGET 200 // 0.2 V
#define MAX_SWEEP_VOLTAGE_TARGET 480 // 4.8 V

#define SWEEP_LENGTH 50.0 // sweep length in ms
#define NUM_SAMPLES_SWEEP 512
#define NUM_SAMPLES_PRESWEEP 64

#define SMOOTH_WINDOW_SIZE 5
#define NUM_SMOOTH_PASSES 10

#define SWEEP_DATA_NUM_2BYTES_PADDING 2

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
static bool adcValueProcessed;
volatile static uint16_t adcValue;

/*
 * Private functions
 */

/**
 * Create the default settings object.
 */
static sweep_settings_t createDefaultSettings() {

    sweep_settings_t settings;
    settings.digiPotGain = INITIAL_GAIN;
    settings.maxDacVoltage = DAC_MIN_VOLTAGE;
    settings.minDacVoltage = DAC_MAX_VOLTAGE;
    settings.dacOffset = DEFAULT_DAC_OFFSET;

    return settings;
}

/**
 * Sets initial values of DigiPot and DAC.
 */
static void initialiseDigiPotAndDac(sweep_settings_t settings) {

    // Set gain on DigiPot
    QT_DIGIPOT_setGain(settings.digiPotGain);

    // Set starting voltage on DAC
    QT_DAC_setOutputValue(settings.minDacVoltage);
}

// TODO check exit flags before doing stuff

/**
 * Remembers value returned by ADC.
 */
static void processAdcValue(uint16_t voltage) {
    adcValue = voltage;
    adcValueProcessed = true;
}

/**
 * Asks ADC for a value according to the retrieve function and waits until value has been returned.
 */
static uint16_t retrieveAdcValue(adc_retrieve_func_t retrieveFunction) {

    adcValueProcessed = false;
    bool deviceReady;

    // Wait until SPI line is free to measure sweep voltage
    do {
        deviceReady = (*retrieveFunction)((adc_read_func_t) processAdcValue);
    }
    while (!deviceReady);

    // Wait until sweep voltage handler is called
    while (!adcValueProcessed);

    uint16_t value = adcValue;
    adcValueProcessed = false;

    return value;
}

/**
 * Sweeps from start DAC voltage to end DAC voltage in SWEEP_LENGTH ms. This function updates the DAC so that it
 * passes through every value.
 */
static sweep_data_t sweep(sweep_settings_t settings, int numSamples) {

    sweep_data_t sweepData;
    uint16_t dacVoltage;
    uint16_t sampleWidth = (settings.maxDacVoltage - settings.maxDacVoltage) * VOLTAGE_MULTIPLE / numSamples;
    uint8_t sampleCounter = 0; // determines when to take readings from the ADC

    sweepData.bufferLength = SWEEP_DATA_NUM_2BYTES_PADDING;

    // Sweep up from start DAC value to end DAC value
    for (dacVoltage = settings.minDacVoltage; dacVoltage <= settings.maxDacVoltage; dacVoltage++) {

        QT_DAC_setOutputValue(dacVoltage);              // set DAC voltage
        sampleCounter++;
        // TODO delay, check PCB temp etc

        if (sampleCounter == sampleWidth) {             // ready to sample
            uint16_t sweepVoltage = retrieveAdcValue(&QT_EADC_measureSweepVoltage);
            uint16_t refVoltage = retrieveAdcValue(&QT_EADC_measureFloatVoltage);
            QT_FRAM_write(sweepVoltage - refVoltage + settings.dacOffset);   // add sweep voltage data
            sweepData.bufferLength++;         // increment buffer length
        }
    }

    // Sweep down from end DAC value to start DAC value TODO make this async
    for (dacVoltage = settings.maxDacVoltage; dacVoltage <= settings.minDacVoltage; dacVoltage--) {
        QT_DAC_setOutputValue(dacVoltage);              // set DAC voltage
    }

    return sweepData;
}

/**
 * Sweeps between sweepSettings.minDacVoltage and sweepSettings.maxDacVoltage and uses the maximum LP voltage to
 * set digiPot resistance for the sweep.
 */
static void setPreSweepDigiPotGain(sweep_settings_t *sweepSettingsPtr) {

    // Set DigiPot to its required gain
    QT_DIGIPOT_setGain(sweepSettingsPtr->digiPotGain);

    // Conduct sweep and retrieve data
    sweep_data_t preSweepData = sweep(*sweepSettingsPtr, NUM_SAMPLES_PRESWEEP);

    // Find max sweep voltage
    uint16_t localMaxSweepVoltage = QT_COM_max(preSweepData.sweepVoltages, preSweepData.bufferLength);

    sweepSettingsPtr->digiPotGain = (float) MAX_SWEEP_VOLTAGE_TARGET / localMaxSweepVoltage;
}

/**
 * Carries out a running smooth on presweep data.
 * TODO do we want a running smooth?
 */
static void smoothPreSweepData(sweep_data_t *preSweepDataPtr) {
    int pass;

    // Smooth the data NUM_SMOOTH_PASSES number of times
    for (pass = 0; pass < NUM_SMOOTH_PASSES; pass++) {

        int i, j;

        // TODO smooth endpoints

        // Smooth middle region
        for (i = SMOOTH_WINDOW_SIZE/2 - 1; i <= preSweepDataPtr->bufferLength - SMOOTH_WINDOW_SIZE/2; i++) {

            uint32_t total = 0;

            for (j = i + 1 - SMOOTH_WINDOW_SIZE/2; j < i - 1 + SMOOTH_WINDOW_SIZE/2; j++) {
                total += preSweepDataPtr->sweepVoltages[j];
            }

            preSweepDataPtr->sweepVoltages[i] = (uint16_t)(total / SMOOTH_WINDOW_SIZE);
        }
    }
}

/**
 * Finds the DAC voltages at the two inflection points in the sweep data.
 */
static void findInflectionPoints(sweep_data_t *preSweepDataPtr, uint16_t *firstInflectionPtr, uint16_t *secondInflectionPtr) {
    // TODO
}

/**
 * Sets the minimum and maximum DAC values for the sweep. SetPreSweepDigiPotGain() should be called first.
 */
static void setPreSweepDacLimits(sweep_settings_t *sweepSettingsPtr) {

    // Set DigiPot to its required resistance
    QT_DIGIPOT_setGain(sweepSettingsPtr->digiPotGain);

    // Conduct sweep - will block until the sweep is completed
    sweep_data_t sweepData = sweep(*sweepSettingsPtr, NUM_SAMPLES_PRESWEEP);

    // Smooth data
    smoothPreSweepData(&sweepData);

    // Find DAC voltages at the inflection points
    uint16_t firstInflection;
    uint16_t secondInflection;
    findInflectionPoints(&sweepData, &firstInflection, &secondInflection);

    // Set DAC limits
    uint16_t targetThirdOfDacRange = secondInflection - firstInflection;
    sweepSettingsPtr->minDacVoltage = firstInflection - targetThirdOfDacRange;
    sweepSettingsPtr->maxDacVoltage = secondInflection + targetThirdOfDacRange;
}

/**
 * Puts the sweep into a safe state.
 */
static void placeSweepInSafeState() {
    // TODO
}

/**
 * Uses feedback from ADC1_2 to find DAC offset. Adds this to settings.
 */
static void adjustForDacOffset(sweep_settings_t *settings) {

    uint16_t adcCurrent = retrieveAdcValue(&QT_EADC_measureSweepCurrent);
    float spVoltageFromAdc = -(55 * adcCurrent)/409600 + 1.65; // TODO make these magic numbers #defines
    float spVoltage = retrieveAdcValue(&QT_EADC_measureSweepVoltage) / VOLTAGE_MULTIPLE;
    settings->dacOffset = spVoltageFromAdc - spVoltage;
}

/*
 * Public functions
 */

/**
 * Calibrates DAC and initialises DigiPot resistance and DAC limits before main sweep.
 */
sweep_settings_t QT_SW_conductPreSweep() {

    sweep_settings_t settings = createDefaultSettings();

    // Set up starting DigiPot and DAC values
    initialiseDigiPotAndDac(settings);

    // Find DAC offset
    adjustForDacOffset(&settings);

    // Set new DigiPot resistance
    setPreSweepDigiPotGain(&settings);

    // Resweep with new DigiPot resistance to find DAC limits
    setPreSweepDacLimits(&settings);

    return settings;
}

/**
 * Conducts a sweep. When sweep data is available, conductSweep() tells the OBC by calling QT_LP_signalSweepDataReady().
 */
sweep_data_t QT_SW_conductSweep(sweep_settings_t settings) {

    // Set DigiPot to its required resistance
    QT_DIGIPOT_setGain(settings.digiPotGain);

    // Conduct sweep
    sweep_data_t sweepData = sweep(settings, NUM_SAMPLES_SWEEP);

    return sweepData;
}
