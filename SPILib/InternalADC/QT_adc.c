/*
 * QT_pcbtemp.c
 *
 *  Created on: 30/01/2019
 *      Author: james
 */

#define CURRENT_OFFSET 0.0
#define CURRENT_SCALE (1.0 / 1023.0)

#define TEMP_R1 56000.0
#define TEMP_R2 56000.0
#define TEMP_R3 732.0
#define TEMP_R4 30000.0

#define TEMP_FINAL_SCALE 4096.0
#define TEMP_RESISTOR_SCALE 6.25
#define TEMP_RESISTOR_OFFSET 1000

#include "QT_adc.h"
#include "driverlib.h"

static volatile int16_t result = 0;
static volatile bool readCompleted = true;

void QT_ADC_initialise() {
    //Set A9 as an input pin.
    //Set A8 as an input pin.
    //Set appropriate module function
    GPIO_setAsPeripheralModuleFunctionInputPin(
                GPIO_PORT_P5,
                GPIO_PIN1,
                GPIO_TERNARY_MODULE_FUNCTION);

    GPIO_setAsPeripheralModuleFunctionInputPin(
                GPIO_PORT_P5,
                GPIO_PIN0,
                GPIO_TERNARY_MODULE_FUNCTION);

    //Initialize the ADC Module
    /*
     * Base Address for the ADC Module
     * Use internal ADC bit as sample/hold signal to start conversion
     * USE MODOSC 5MHZ Digital Oscillator as clock source
     * Use default clock divider of 1
     */
    ADC_init(ADC_BASE,
             ADC_SAMPLEHOLDSOURCE_SC,
             ADC_CLOCKSOURCE_ADCOSC,
             ADC_CLOCKDIVIDER_1);

    ADC_enable(ADC_BASE);

    /*
     * Base Address for the ADC Module
     * Sample/hold for 16 clock cycles
     * Do not enable Multiple Sampling
     */
    ADC_setupSamplingTimer(ADC_BASE,
                           ADC_CYCLEHOLD_16_CYCLES,
                           ADC_MULTIPLESAMPLESDISABLE);

    ADC_clearInterrupt(ADC_BASE,
                       ADC_COMPLETED_INTERRUPT);

    //Enable Memory Buffer interrupt
    ADC_enableInterrupt(ADC_BASE,
                        ADC_COMPLETED_INTERRUPT);
}

/**
 * This function reads the temperature of the PCB in C, this function will block for a small amount of time while the tempurature is read (16 cycles)
 */
float QT_ADC_readTemperature() {
    // Ensure that there is not a read running
    while(!readCompleted) {  }

    ADC_disableConversions(ADC_BASE, ADC_COMPLETECONVERSION);
    ADC_configureMemory(ADC_BASE,
                        ADC_INPUT_A9,
                        ADC_VREFPOS_AVCC,
                        ADC_VREFNEG_AVSS);

    readCompleted = false;
    ADC_startConversion(ADC_BASE, ADC_SINGLECHANNEL);

    while(!readCompleted) {  }

    // Equation from LP control docs. k is the constant offset in the equation
    // while m is the complicated multicand.
    float k = TEMP_R4 / TEMP_R2;
    float m = TEMP_R4 * (1.0 / TEMP_R3 + 1.0 / TEMP_R2) + 1.0;

    float x = (k - result / TEMP_FINAL_SCALE) / m;

    float RT = TEMP_R1 * x / (1 - x);

    return (RT - TEMP_RESISTOR_OFFSET) / TEMP_RESISTOR_SCALE;
}

/**
 * This function reads the battery current, this function will block for a small amount of time while the current is read (16 cycles)
 */
float QT_ADC_readCurrent() {
    // Ensure that there is not a read running
    while(!readCompleted) {  }

    ADC_disableConversions(ADC_BASE, ADC_COMPLETECONVERSION);
    ADC_configureMemory(ADC_BASE,
                        ADC_INPUT_A8,
                        ADC_VREFPOS_AVCC,
                        ADC_VREFNEG_AVSS);

    readCompleted = false;
    ADC_startConversion(ADC_BASE, ADC_SINGLECHANNEL);

    while(!readCompleted) {  }

    return (result + CURRENT_OFFSET) * CURRENT_SCALE;
}

//ADC10 interrupt service routine
#pragma vector=ADC_VECTOR
__interrupt
void ADC_ISR() {
    switch (__even_in_range(ADCIV,12)){
    case  0: break; //No interrupt
    case  2: break; //conversion result overflow
    case  4: break; //conversion time overflow
    case  6: break; //ADC10HI
    case  8: break; //ADC10LO
    case 10: break; //ADC10IN
    case 12:        //ADC10IFG0
        //(Automatically clears ADC10IFG0 by reading memory buffer)
        result = ADC_getResults(ADC_BASE);
        readCompleted = true;

        break;
    default: break;
    }
}
