#include "driverlib.h"
#include <stdbool.h>
#include "msp430fr2355.h"

#define EXIT 0xFF
#define OBC_SPI_TEST 1
#define SWITCH_TEST 2
#define BURN_WIRE_TEST 3
#define POWER_TEST 4
#define GUARD_SWITCH_TEST 5
#define TEMP_SENSOR_TEST 6
#define CURRENT_TEST 7
#define CONTACT_SWITCH_TEST 8

volatile int test_state = 0;

volatile int d1 = 0;
volatile int d2 = 0;
volatile int count1 = 0;
volatile int count2 = 0;

static volatile int16_t result = 0;
static volatile bool readCompleted = true;

//#define DCOCTL 0x056
//#define BCSCTL1 0x057
//#define BCSCTL2 0x058
//#define BCSCTL3 0x053

void turnoff_GPIOs(void) {
    P1DIR = 0x00;
    P1OUT = 0x00;
    P1REN = 0x00;
    P2DIR = 0x00;
    P2OUT = 0x00;
    P2REN = 0x00;
    P3DIR = 0x00;
    P3OUT = 0x00;
    P3REN = 0x00;
    P4DIR = 0x00;
    P4OUT = 0x00;
    P4REN = 0x00;
    P5DIR = 0x00;
    P5OUT = 0x00;
    P5REN = 0x00;
    P6DIR = 0x00;
    P6OUT = 0x00;
    P6REN = 0x00;
}

void setup_LED_interrupts(void) {
    TB1CCTL0 = CCIE;                            // CCR0 interrupt enabled
    TB0CCTL0 = CCIE;

    TB0CCR0 = 55000;
    TB1CCR0 = 6000;

    TB0CTL = TBSSEL_1 + ID_2;                 // SMCLK, contmode,
    TB1CTL = TBSSEL_1 + ID_0;

    P4DIR |= 0x03;
}

void QT_IADC_initialise(int adc_port, int adc_pin) {
    //Set A9 as an input pin.
    //Set A8 as an input pin.
    //Set appropriate module function
//    GPIO_PORT_P5
//    GPIO_PIN0
    GPIO_setAsPeripheralModuleFunctionInputPin(
                adc_port,
                adc_pin,
                GPIO_TERNARY_MODULE_FUNCTION);

//    GPIO_setAsPeripheralModuleFunctionInputPin(
//                GPIO_PORT_P5,
//                GPIO_PIN0,
//                GPIO_TERNARY_MODULE_FUNCTION);

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

    ADC_setResolution(ADC_BASE, ADC_RESOLUTION_12BIT);

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

    setup_LED_interrupts();
}

float IADC_read(int adc_select) {
    // Ensure that there is not a read running
    while(!readCompleted) {  }

    ADC_disableConversions(ADC_BASE, ADC_COMPLETECONVERSION);
    ADC_configureMemory(ADC_BASE,
                        adc_select,
                        ADC_VREFPOS_AVCC,
                        ADC_VREFNEG_AVSS);

    readCompleted = false;
    ADC_startConversion(ADC_BASE, ADC_SINGLECHANNEL);

    while(!readCompleted) {  }

    // Equation from LP control docs. k is the constant offset in the equation
    // while m is the complicated multicand.
    return result;
}

void general_setup(void) {
    WDT_A_hold(WDT_A_BASE);

    CSCTL0 = 0;
    CSCTL1 = DCORSEL_3;
    CSCTL0 = DCO0;
    PM5CTL0 &= ~LOCKLPM5;

    turnoff_GPIOs();

    TB2CCTL0 = CCIE;
    TB2CTL = TBSSEL_1 + ID_3;
    __bis_SR_register(GIE);
    __no_operation();
}

void obc_spi_test(void) {
    P4DIR = 0x0F;
    P4OUT |= 0x0F;
    TB2CTL |= MC_2;
}

bool switch_button(void){
    P4DIR |= BIT2;
    P4OUT |= BIT2;
    P4REN |= BIT3;
    int button = P4IN & BIT3;
    if (button > 0) {
        return true;
    } else {
        return false;
    }
}

void switch_test(void) {
    P4DIR |= BIT1;
    if (switch_button()) {
        P4OUT |= BIT1;
    } else {
        P4OUT &= ~(BIT1);
    }
}

void burn_wire_test(void) {
    P1DIR |= (BIT1 | BIT2);
    switch_test();
    if (switch_button()) {
        P1OUT |= (BIT1 | BIT2);
    } else {
        P1OUT &= ~(BIT1 | BIT2);
    }
}

void power_test(void) {
    P3DIR |= BIT2;
    switch_test();
    if (switch_button()) {
        P3OUT |= BIT2;
    } else {
        P3OUT &= ~BIT2;
    }
}

void guard_switch_test(void) {
    //TODO should +- 16V power be turned on here?
    switch_test();
    power_test();
    if (switch_button()) {
        P3DIR |= BIT3;
        P3OUT |= BIT3;
    } else {
        P3DIR &= ~BIT3;
        P3OUT &= ~BIT3;
    }
}

void temp_sense_test(void) {
    if (switch_button()) {
//        P4OUT |= (BIT0 | BIT1);
        IADC_read(ADC_INPUT_A9);
        TB0CTL |= MC_1;
        TB1CTL |= MC_1;
    } else {
        TB0CTL &= ~MC_1;
        TB1CTL &= ~MC_1;
        P4OUT &= ~(BIT0 | BIT1);
    }
}

void current_test(void) {
//    P4OUT |= (BIT0 | BIT1);
    P3OUT &= ~(BIT4);
    TB0CTL |= MC_1;
    TB1CTL |= MC_1;
    IADC_read(ADC_INPUT_A8);
    if (switch_button()) {
        P3DIR |= (BIT2);
        P3OUT |= (BIT2);
    } else {
        P3OUT &= ~(BIT2);
    }
}

void contact_switch_test(void) {
    P3REN |= (BIT0 | BIT1);
    P3OUT |= (BIT0 | BIT1);
    P4DIR |= BIT0;
    int state = (P3IN & BIT0);
    int state1 = (P3IN & BIT1);
    if ( (state > 0)) {
        P4OUT |= BIT0;
    } else {
        P4OUT &= ~BIT0;
    }
}

void exit_main(void) {
    turnoff_GPIOs();
    __bis_SR_register(CPUOFF + GIE + LPM4);//
}

void main(void)
{
    general_setup();

    test_state = BURN_WIRE_TEST;

//    P4DIR |= BIT0;
//    P4OUT |= BIT0;

    switch (test_state) {
    case    TEMP_SENSOR_TEST:
        QT_IADC_initialise(GPIO_PORT_P5, GPIO_PIN1);
        break;
    case    CURRENT_TEST:
        QT_IADC_initialise(GPIO_PORT_P5, GPIO_PIN0);
        break;
    }

    while(1)
    {
        switch (test_state) {
        case    OBC_SPI_TEST:
            obc_spi_test();
            break;
        case    SWITCH_TEST:
            switch_test();
            break;
        case    BURN_WIRE_TEST:
            burn_wire_test();
            break;
        case    POWER_TEST:
            power_test();
            break;
        case    GUARD_SWITCH_TEST:
            guard_switch_test();
            break;
        case    TEMP_SENSOR_TEST:
            temp_sense_test();
            break;
        case    CURRENT_TEST:
            current_test();
            break;
        case    CONTACT_SWITCH_TEST:
            contact_switch_test();
            break;
        case    EXIT:
            exit_main();
            break;
        }
    }

}

#pragma vector = TIMER0_B0_VECTOR
__interrupt void timer_b0(void) {
    d1 = (result/400);
    d2 = (result/40)%10;
}

#pragma vector = TIMER1_B0_VECTOR
__interrupt void timer_b1(void) {
    //TODO replace
    if (count1< 2*d1) {
        P4OUT ^= 0x02;
        count1 += 1;
    } else if (count2 < 2*d2) {
        P4OUT ^= 0x01;
        count2 += 1;
    } else {
        count1 = 0;
        count2 = 0;
        P4OUT &= ~0x03;
    }
//    TB1CCR0 += 50000;
}

#pragma vector = TIMER2_B0_VECTOR
__interrupt void timer_b2(void) {
//    P1DIR |= (BIT0 | BIT1); //TODO REMOVE THIS
//    P1OUT = 0x03;
    __delay_cycles(30000000);
    P4OUT = 0x00;
    exit_main();
}

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
        result = (ADC_getResults(ADC_BASE));
        readCompleted = true;
        break;
    default: break;
    }
}
