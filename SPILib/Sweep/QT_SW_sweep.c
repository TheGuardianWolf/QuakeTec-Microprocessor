///*
// * Includes
// */
//#include "Sweep/QT_SW_sweep.h"
//
///*
// * Defines
// */
//
//#define INITIAL_GAIN 2.0
//
//#define VOLTAGE_MULTIPLE 1000
//#define DAC_MIN_VOLTAGE (0 * VOLTAGE_MULTIPLE)
//#define DAC_MAX_VOLTAGE (5 * VOLTAGE_MULTIPLE)
//#define MIN_SWEEP_VOLTAGE_TARGET 200 // 0.2 V
//#define MAX_SWEEP_VOLTAGE_TARGET 480 // 4.8 V
//
//// The sweep length in ms
//#define SWEEP_LENGTH 50.0
//
//#define NUM_VALUES_12_BITS 4096
//#define NUM_VALUES_10_BITS 1024
//#define MAX_VALUE_16_BITS 65535
//
//#define SWEEP_RESOLUTION_DIVISOR 1
//#define PRESWEEP_RESOLUTION_DIVISOR 8
//
//#define SMOOTH_WINDOW_SIZE 5
//#define NUM_SMOOTH_PASSES 10
//
///*
// * Type definitions
// */
//
///*
// * Private variables
// */
//
///*
// * Private functions
// */
//
//// TODO move the following functions into the appropriate modules
//
///**
// * Converts voltage used internally by MCU into a uint16 value for the DAC to use.
// */
//static uint16_t convertToDacVoltage(uint16_t voltage) {
//    return voltage; // TODO
//}
//
///**
// * Converts voltage used by DAC into the corresponding voltage used internally by the MCU.
// */
//static uint16_t convertFromDacVoltage(uint16_t voltage) {
//    return voltage; // TODO
//}
//
///**
// * Converts voltage used internally by MCU into a uint16 value for the ADC to use.
// */
//static uint16_t convertToAdcVoltage(uint16_t voltage) {
//    return voltage; // TODO
//}
//
///**
// * Converts voltage used by ADC into the corresponding voltage used internally by the MCU.
// */
//static uint16_t convertFromAdcVoltage(uint16_t voltage) {
//    return voltage; // TODO
//}
//
//// TODO move the following functions (max and min) into common module
//
///**
// * Finds the maximum value in an array.
// */
//static uint16_t max(uint16_t values [], uint16_t length) {
//    int i;
//    uint16_t currentMax = values[0];
//
//    for (i = 1; i < length; i++) {
//        if (values[i] > currentMax) {
//            currentMax = values[i];
//        }
//    }
//
//    return currentMax;
//}
//
///**
// * Finds the minimum value in an array.
// */
//static uint16_t min(uint16_t values [], uint16_t length) {
//    int i;
//    uint16_t currentMin = values[0];
//
//    for (i = 1; i < length; i++) {
//        if (values[i] < currentMin) {
//            currentMin = values[i];
//        }
//    }
//
//    return currentMin;
//}
//
///**
// * Create the default settings object.
// */
//static sweep_settings_t createDefaultSettings() {
//    sweep_settings_t settings;
//    settings.digiPotGain = INITIAL_GAIN;
//    settings.maxDacVoltage = DAC_MIN_VOLTAGE;
//    settings.minDacVoltage = DAC_MAX_VOLTAGE;
//
//    return settings;
//}
//
///**
// * Sets initial values of DigiPot and DAC.
// */
//static void initialiseDigiPotAndDac(sweep_settings_t settings) {
//    // Set resistance on DigiPot
//    QT_DIGIPOT_setGain(settings.digiPotGain);
//
//    // Set starting voltage on DAC
//    QT_DAC_setOutputValue(settings.minDacVoltage);
//}
//
//// TODO check exit flags before doing stuff
//// TODO make things set in interrupts volatile
//
///**
// * Sweeps from start DAC voltage to end DAC voltage in SWEEP_LENGTH ms. This function will update the DAC so that is passes through every value.
// */
//static void sweep(sweep_t *sweepPtr, uint16_t startDacValue, uint16_t endDacValue) {
//
//
//    // Wait until sweep voltage data can be overwritten
//    while (sweepPtr->sweepVoltages.positionPtr - &(sweepPtr->sweepVoltages.voltages) >= SWEEP_DATA_BUFFER_SIZE);
//
//    sweepPtr->sweepData.bufferLength = SWEEP_DATA_NUM_2BYTES_PADDING;
//
//    // Find end DAC voltage
//    uint16_t maxEndVoltage = startDacVoltage + SWEEP_DATA_BUFFER_SIZE - SWEEP_DATA_NUM_2BYTES_PADDING - 1;
//    if (endDacVoltage - startDacVoltage > maxEndVoltage) {
//        endDacVoltage = maxEndVoltage;
//    }
//
//    uint16_t dacVoltage;
//
//    // Sweep from start to end DAC voltages in increments on the resolution divisor
//    for (dacVoltage = startDacVoltage; dacVoltage <= endDacVoltage; dacVoltage += resolutionDivisor) {
//        // TODO wait for new readings from ADCs
//        //QT_DAC_setOutputValue(dacVoltage);    // set DAC voltage
//        uint16_t sweepVoltage = 0;              // TODO retrieve from ADC1_1
//        uint16_t refVoltage = 0;                // TODO retrieve from ADC1_2
//
//        *(*(sweepPtr->sweepVoltages.positionPtr)) = sweepVoltage - refVoltage; // add sweep voltage data
//        sweepPtr->sweepVoltages.positionPtr++; // move current voltage pointer
//        sweepPtr->sweepData.bufferLength++; // increment buffer length
//    }
//}
//
///**
// * Sweeps between sweepSettings.minDacVoltage and sweepSettings.maxDacVoltage and uses the maximum LP voltage to
// * set digiPot resistance for the sweep.
// */
//static void setPreSweepDigiPotResistance() {
//
//    // Initialise min and max sweep voltages
//    uint16_t maxSweepVoltage = 0;
//
//    uint16_t startDacVoltage = sweepSettings.minDacVoltage;
//    int i;
//
//    for (i = startDacVoltage; i < sweepSettings.maxDacVoltage; i += SWEEP_DATA_BUFFER_SIZE) {
//
//        // Set DigiPot to its required resistance
//        setDigiPotResistance(sweepSettings.digiPotResistance);
//
//        // Conduct sweep and retrieve data
//        sweep(currentSweepPtr, startDacVoltage, sweepSettings.maxDacVoltage, PRESWEEP_RESOLUTION_DIVISOR);
//        sweep_data_t *preSweepDataPtr = QT_SW_retrieveSweepData();
//
//        // Improve max sweep voltage
//        uint16_t localMaxSweepVoltage = max(currentSweepPtr->sweepVoltages.voltages, currentSweepPtr->sweepData.bufferLength);
//        if (localMaxSweepVoltage > maxSweepVoltage) {
//            maxSweepVoltage = localMaxSweepVoltage;
//        }
//    }
//
//    uint16_t maxGain = MAX_SWEEP_VOLTAGE_TARGET / maxSweepVoltage;
//    sweepSettings.digiPotResistance = DIGIPOT_GAIN_FACTOR / (maxGain - 1);
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
//                total += (*(preSweepDataPtr->sweepVoltages))[j];
//            }
//
//            (*(preSweepDataPtr->sweepVoltages))[i] = (uint16_t)(total / SMOOTH_WINDOW_SIZE);
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
// * Sets the minimum and maximum DAC values for the sweep. SetPreSweepDigiPotResistance() should be called first.
// */
//static void setPreSweepDacLimits() {
//
//    // Set DigiPot to its required resistance
//    setDigiPotResistance(sweepSettings.digiPotResistance);
//
//    // Conduct sweep - will block until the sweep is completed
//    sweep(currentSweepPtr, sweepSettings.minDacVoltage, sweepSettings.maxDacVoltage, PRESWEEP_RESOLUTION_DIVISOR);
//
//    // Smooth data
//    sweep_data_t *preSweepDataPtr = QT_SW_retrieveSweepData();
//    smoothPreSweepData(preSweepDataPtr);
//
//    // Find DAC voltages at the inflection points
//    uint16_t firstInflection;
//    uint16_t secondInflection;
//    findInflectionPoints(preSweepDataPtr, &firstInflection, &secondInflection);
//
//    // Set DAC limits
//    uint16_t targetThirdOfDacRange = secondInflection - firstInflection;
//    sweepSettings.minDacVoltage = firstInflection - targetThirdOfDacRange;
//    sweepSettings.maxDacVoltage = secondInflection + targetThirdOfDacRange;
//}
//
///*
// * Public functions
// */
//
///**
// * Calibrates DAC and initialises DigiPot resistance and DAC limits before main sweep.
// */
//void QT_SW_conductPreSweep() {
//    // TODO health checks
//
//    sweep_settings_t settings = createDefaultSettings();
//
//    // Set up starting DigiPot and DAC values
//    initialiseDigiPotAndDac(settings);
//
//    // TODO find DAC offset
//
//    // Set new DigiPot resistance
//    setPreSweepDigiPotResistance();
//
//    // Resweep with new DigiPot resistance to find DAC limits
//    setPreSweepDacLimits();
//}
//
///**
// * Returns a pointer to a sweep_settings_t struct containing the digiPot resistance and max and min DAC values
// * for the sweep.
// */
//sweep_settings_t QT_SW_retrieveSweepSettings() {
//    return sweepSettings; // TODO what if settings are not ready?
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
// * Conducts a sweep. When sweep data is available, conductSweep() tells the OBC by calling QT_LP_signalSweepDataReady().
// */
//sweep_data_t QT_SW_conductSweep(sweep_settings_t settings) {
//
//    uint16_t startDacVoltage = sweepSettings.minDacVoltage;
//    int i;
//
//    for (i = startDacVoltage; i < sweepSettings.maxDacVoltage; i += SWEEP_DATA_BUFFER_SIZE) {
//
//        // Set DigiPot to its required resistance
//        setDigiPotResistance(sweepSettings.digiPotResistance);
//
//        // Conduct sweep - will block until the sweep data buffer is allowed to be filled
//        sweep(currentSweepPtr, startDacVoltage, sweepSettings.maxDacVoltage, SWEEP_RESOLUTION_DIVISOR);
//
//        // Signal that data is ready
//        // QT_LP_signalSweepDataReady(); TODO uncomment
//    }
//}
