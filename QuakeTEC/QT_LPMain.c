#include "driverlib.h"
#include "InternalADC/QT_adc_internal.h"
#include "ExternalADC/QT_EADC.h"
#include "SpiLib/QT_SPI_SpiLib.h"
#include "BurnWire/QT_BW_BurnWire.h"
#include "Sweep/QT_SW_sweep.h"
#include "Timer/QT_timer.h"
#include "PowerControl/QT_PWR_power.h"
#include "PL_Protocol.h"
#include "Digipot/QT_DIGIPOT.h"
#include "SpiLib/QT_MSP430_SPIMaster.h"
#include "Digipot/QT_AD5292.h"
#include "OBCInterface/QT_OBC_Interface.h"

volatile uint16_t ERROR_STATUS = 0;
volatile bool exitCommand = false;

/**
 * Setup the libraries and IO pins.
 */
void initialise() {
    volatile uint32_t aclk_speed;
    volatile uint32_t mclk_speed;
    volatile uint32_t smclk_speed;
    // Stop watchdog timer
    WDT_A_hold(WDT_A_BASE);

    // Disable the GPIO power-on default high-impedance mode
    // to activate previously configured port settings
    PMM_unlockLPM5();

    CS_initClockSignal(CS_ACLK, CS_VLOCLK_SELECT, CS_CLOCK_DIVIDER_1);

    CS_initFLLSettle(
            4000,
            122
    );

    // Configure everything as pulldown input
    GPIO_setAsInputPinWithPullDownResistor(GPIO_PORT_P1, GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN2 | GPIO_PIN3);
    GPIO_setAsInputPinWithPullDownResistor(GPIO_PORT_P2, 0xff);
    GPIO_setAsInputPinWithPullDownResistor(GPIO_PORT_P3, 0xff);
    GPIO_setAsInputPinWithPullDownResistor(GPIO_PORT_P4, 0xff);
    GPIO_setAsInputPinWithPullDownResistor(GPIO_PORT_P5, GPIO_PIN0 | GPIO_PIN1);

    //Configure default states for other GPIOs
    exitCommand = false;
    ERROR_STATUS = 0;

    SPIMaster_Init();

    QT_PWR_turnOffGuard();
    QT_PWR_turnOn16V();

    QT_BW_reset();

    QT_IADC_initialise();

    QT_DIGIPOT_init();

    QT_DAC_reset();

    QT_OBCSPI_init();

    QT_OBC_Interface_init();

    aclk_speed = CS_getACLK();
    mclk_speed = CS_getMCLK();
    smclk_speed = CS_getSMCLK();
    __enable_interrupt();

    QT_EADC_initialise();
}

void setupDeploymentTest() {
    P1DIR |= BIT1;
//    P1OUT |= BIT1;

    //Pin interrupt
    P2REN |= BIT0;
    P2OUT |= BIT0;
    P2IE |= BIT0;
    P2IES |= BIT0;
    P2IFG &= ~BIT0;
}

static volatile int pin_flag=1;
void main(void) {
    initialise();

    while(true) {
        QT_OBC_Interface_commandLoop();
    }
}



