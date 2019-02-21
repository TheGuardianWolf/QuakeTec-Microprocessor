/*
 * Includes
 */
#include "Sweep/QT_SW_sweep.h"

/*
 * Defines
 */
#define DIGIPOT_INITIAL_RESISTANCE 9800
#define DIGIPOT_MAX_RESISTANCE 20000
#define DAC_MIN_VOLTAGE 0
#define DAC_MAX_VOLTAGE 5
#define NUM_VALUES_12_BITS 4096
#define NUM_VALUES_10_BITS 1024
#define MAX_VALUE_16_BITS 65535
#define DIGIPOT_WRITE_COMMAND 0b0001
#define SWEEP_DATA_NUM_BYTES_PADDING 2

/*
 * Private variables
 */
static uint16_t digiPotResistance = DIGIPOT_INITIAL_RESISTANCE;
static float minDacVoltage = DAC_MIN_VOLTAGE;
static float maxDacVoltage = DAC_MAX_VOLTAGE;

static uint16_t sweepVoltages1 [SWEEP_DATA_BUFFER_SIZE];
static uint16_t **sweepVoltages1PositionPtr;

static uint16_t sweepVoltages2 [SWEEP_DATA_BUFFER_SIZE];
static uint16_t **sweepVoltages2PositionPtr;

/*
 * Private functions
 */

// TODO remove floats

/**
 * Converts voltage for DAC into a uint16 value for DAC to use.
 */
static uint16_t convertVoltageToUint16(float voltage) {
    return (uint16_t) (voltage * NUM_VALUES_12_BITS / (DAC_MAX_VOLTAGE - DAC_MIN_VOLTAGE));
}

/**
 * Converts uint16 value used by DAC into corresponding voltage.
 */
static float convertUint16ToVoltage(uint16_t voltage) {
    return (DAC_MAX_VOLTAGE - DAC_MIN_VOLTAGE) * (float) voltage / NUM_VALUES_12_BITS;
}

/**
 * Converts a resistance for the digipot into the value to set.
 */
static uint16_t convertResistanceToUint10(uint16_t resistance) {
    return (uint16_t) (resistance * NUM_VALUES_10_BITS / DIGIPOT_MAX_RESISTANCE);
}

/**
 * Sets the resistance of the DigiPot.
 */
static void setDigiPotResistance(uint16_t resistance) {
    //QT_DIGIPOT_setControlBits(DIGIPOT_WRITE_COMMAND); // TODO uncomment
    //QT_DIGIPOT_setDataBits(convertResistanceToUint10(resistance)); // TODO uncomment
}

/**
 * Sets initial values of DigiPot and Dac.
 */
static void initialiseDigiPotAndDac() {

    // Setup starting DigiPot resistance
    digiPotResistance = DIGIPOT_INITIAL_RESISTANCE;

    // Set digipot resistance
    setDigiPotResistance(digiPotResistance);

    // Setup starting DAC voltage
    minDacVoltage = DAC_MIN_VOLTAGE;
    maxDacVoltage = DAC_MAX_VOLTAGE;
}

/**
 * Sweeps between minimum and maximum voltages with the digipot's resistance set to digiPotResistance.
 */
static sweep_data_t sweep(float minDacVoltage, float maxDacVoltage, uint16_t digiPotResistance) {

    // Set DigiPot resistance
    setDigiPotResistance(digiPotResistance);

    uint16_t startDacVoltage = convertVoltageToUint16(minDacVoltage);
    uint16_t endDacVoltage = convertVoltageToUint16(maxDacVoltage);

    uint16_t dacVoltage = 0; // TODO set
    sweep_data_t preSweepData = {0};

    for (dacVoltage = startDacVoltage; dacVoltage <= endDacVoltage; dacVoltage++) {
        //QT_DAC_setOutputValue(dacVoltage);    // set DAC voltage TODO wait for value to be set
        uint16_t sweepVoltage = 0;              // TODO retrieve from ADC1_1
        uint16_t refVoltage = 0;                // TODO retrieve from ADC1_2
        preSweepData.sweepVoltages[dacVoltage - startDacVoltage] = sweepVoltage - refVoltage;
    }

    return preSweepData;
}

/*
 * Public functions
 */

/**
 * Initialises DigiPot and DAC before main sweep, calibrates DAC and collects data.
 */
void QT_SW_conductPreSweep() {
    // TODO health checks

    initialiseDigiPotAndDac();

    // TODO calibrate DAC

    sweep_data_t preSweepData = {0};
    uint16_t maxSweepVoltage = 0;
    uint16_t minSweepVoltage = MAX_VALUE_16_BITS;

    while (true) {
        preSweepData = sweep(minDacVoltage, maxDacVoltage, digiPotResistance);
        maxSweepVoltage = 0; // TODO
        minSweepVoltage = 0; // TODO
    }
}

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

/*
 * Interrupts
 */


