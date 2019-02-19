/*
 * QT_pcbtemp.c
 *
 *  Created on: 30/01/2019
 *      Author: james
 */

#define OFFSET 0.0
#define SCALE (1.0 / 1023.0);

#include "QT_pcbtemp.h"
#include "driverlib.h"

static volatile int16_t result = 0;
static volatile bool hasRead = false;

void QT_TEMP_initialise() {
    //Set A9 as an input pin.
    //Set appropriate module function
    GPIO_setAsPeripheralModuleFunctionInputPin(
            GPIO_PORT_P5,
            GPIO_PIN1,
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

    //Configure Memory Buffer
    /*
     * Base Address for the ADC Module
     * Use input A9
     * Use positive reference of AVcc
     * Use negative reference of AVss
     */
    ADC_configureMemory(ADC_BASE,
                        ADC_INPUT_A9,
                        ADC_VREFPOS_AVCC,
                        ADC_VREFNEG_AVSS);

    ADC_clearInterrupt(ADC_BASE,
                       ADC_COMPLETED_INTERRUPT);

    //Enable Memory Buffer interrupt
    ADC_enableInterrupt(ADC_BASE,
                        ADC_COMPLETED_INTERRUPT);
}

/**
 * This function reads the temperature of the PCB, this function will block for a small amount of time while the tempurature is read (16 cycles)
 */
float QT_TEMP_readTemperature() {
    hasRead = false;
    ADC_startConversion(ADC_BASE, ADC_REPEATED_SINGLECHANNEL);

    while(!hasRead) {
        // Spin loop
    }

    return (result + OFFSET) * SCALE;
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
        hasRead = true;

        break;
    default: break;
    }
}
