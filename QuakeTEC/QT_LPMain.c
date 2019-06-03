#include "QT_LPMain.h"

extern volatile bool exitCommand;
extern volatile PL_Command_t currentCommand;
extern volatile bool commandRunning;
extern volatile uint16_t ERROR_STATUS;

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

//    CSCTL4 = 0;
//    CSCTL4 |= BIT9 + SELMS_0;

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



//    P3DIR |= (BIT4 | BIT3 | BIT2);
//    P3REN |= (BIT1 | BIT0);
//
//    P1DIR |= (BIT2 | BIT1);
//
//    P2DIR |= (BIT6 | BIT5 | BIT4);
//    P2OUT |= (BIT6 | BIT5 | BIT4);
//    P2REN &= ~(BIT7);
//
//    P4DIR |= (BIT6 | BIT5 | BIT4 | BIT2);
//    P4OUT |= (BIT7 | BIT4);
//    P4REN |= BIT7;
//    P4REN &= ~(BIT3 | BIT1 | BIT0);
    exitCommand = false;
    currentCommand = -1;
    commandRunning = false;
    ERROR_STATUS = 0;

    SPIMaster_Init();

    QT_PWR_turnOffGuard();
    QT_PWR_turnOn16V();

    QT_BW_reset();

    QT_IADC_initialise();
    QT_SPI_initialise();

    QT_DIGIPOT_init();

    QT_DAC_reset();

    // Enable interrupts

    aclk_speed = CS_getACLK();
    mclk_speed = CS_getMCLK();
    smclk_speed = CS_getSMCLK();
    __enable_interrupt();
    __no_operation();

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
    startListening();

//    uint16_t d = QT_COM_maxByteArray(a, 20);
//    P2REN |= BIT0;
//    P2OUT |= BIT0;
//    P2IE |= BIT0;
//    P2IES |= BIT0;
//    P2IFG &= ~BIT0;

//    P3DIR = BIT2;
//    P3OUT = BIT2;
//    P1DIR |= BIT2;
//    P1OUT |= BIT2;
    byte data[2] = {0xAA, 0xAA};

//    while (true) {
////        QT_SW_getPlasmaData();
////        asdf = QT_EADC_getAdcValue(ADC1);
//        QT_SPI_transmit(
//                data,
//                2,
//                &OBC,
//                NULL);
//        QT_SPI_listenToMaster();
//    }


//    QT_DAC_reset();

    while(true) {

//        QT_EADC_measureSweepCurrent(&adcHandler);
//         Wait until a command has been queued
        while(!commandRunning);
//         These command functions are blocking.
        PL_Command_t command = currentCommand;
        switch(currentCommand) {
        case PL_COMMAND_STOP:
        {
            int a =1;
        }
            break;
        case PL_COMMAND_POWER_OFF:
            QT_PWR_turnOff16V();
            queueEvent(PL_EVENT_PROBE_UNPOWERED);
            break;
        case PL_COMMAND_POWER_ON:
            QT_PWR_turnOn16V();
            queueEvent(PL_EVENT_PROBE_POWERED);
            break;
        case PL_COMMAND_SWEEP:
            queueEvent(PL_EVENT_SWEEP_STARTED);
            QT_SW_getPlasmaData();
            queueEvent(PL_EVENT_SWEEP_COMPLETE);
            break;
        case PL_COMMAND_DEPLOY:
            queueEvent(PL_EVENT_DEPLOY_STARTED);
            QT_BW_deploy();
            queueEvent(PL_EVENT_DEPLOY_COMPLETE);
            break;
        case PL_COMMAND_DEPLOY_SP:
            queueEvent(PL_EVENT_DEPLOY_STARTED);
            QT_BW_deploySP();
            queueEvent(PL_EVENT_DEPLOY_COMPLETE);
            break;
        case PL_COMMAND_DEPLOY_FP:
            queueEvent(PL_EVENT_DEPLOY_STARTED);
            QT_BW_deployFP();
            queueEvent(PL_EVENT_DEPLOY_COMPLETE);
            break;
        case PL_COMMAND_CLEAR_ERRORS:
            ERROR_STATUS = 0;
        default:
            break;
        }

        commandRunning = false;
    }
}



