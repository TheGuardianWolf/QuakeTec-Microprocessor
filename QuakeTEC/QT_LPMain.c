#include <InternalADC/QT_adc_internal.h>
#include "driverlib.h"

#include "ExternalADC/QT_adc_external.h"
#include "SpiLib/QT_SPI_Protocol.h"
#include "SpiLib/QT_SPI_SpiLib.h"
#include "QT_LPMain.h"
#include "BurnWire/QT_BW_BurnWire.h"

#include "Timer/QT_timer.h"

#define EVENT_QUEUE_LENGTH 256
#define EVENT_QUEUE_HEADER_LENGTH 2

// Event buffer code. The buffer is on
byte eventQueue[EVENT_QUEUE_LENGTH + EVENT_QUEUE_HEADER_LENGTH];
int eventQueueStart = EVENT_QUEUE_LENGTH;
volatile bool finishedSendingEvents = true;
volatile f_flags F_FLAGS = {0};

// Sweep values
uint16_t dacValue;

uint8_t digipotControl;
uint8_t digipotData;

// Command systems
volatile bool exitCommand = false;

/** The value of this variable is undefined if commandRunning = false */
volatile PL_Command_t currentCommand;
volatile bool commandRunning = false;

/**
 * Setup the libraries and IO pins.
 */
void initialise() {
    volatile uint32_t aclk_speed;
    volatile uint32_t mclk_speed;
    volatile uint32_t smclk_speed;
    // Stop watchdog timer
    WDT_A_hold(WDT_A_BASE);

//    CSCTL4 = 0;
//    CSCTL4 |= BIT9 + SELMS_0;

//    CS_initClockSignal(CS_ACLK, CS_VLOCLK_SELECT, CS_CLOCK_DIVIDER_1);

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

    QT_IADC_initialise();
    QT_SPI_initialise();

     QT_EADC_initialise();

    // Disable the GPIO power-on default high-impedance mode
    // to activate previously configured port settings
    PMM_unlockLPM5();

    //Set Aclk to use VLO internal oscillator 10kHz
//    CSCTL4 &= ~(BIT9|BIT8);

    // Enable interrupts

//    aclk_speed = CS_getACLK();
//    mclk_speed = CS_getMCLK();
//    smclk_speed = CS_getSMCLK();
    __enable_interrupt();
}

/**
 * Adds an event to the list.
 *
 * TODO Shout at the OBC if we overflow
 **/
void queueEvent(PL_Event_t event) {
    if(eventQueueStart <= 0 || !finishedSendingEvents) {
        // Overflow. This should not occur givent he OBC does not send more than 256 commands.
        return;
    }

    eventQueue[--eventQueueStart] = (byte) event;
}

void finishTransmission(bool unused) {
    QT_SPI_listenToMaster();
}

void flagEventsFinishedSending(bool unused) {
    finishedSendingEvents = true;

    finishTransmission(unused);
}

void sendEvents() {
    eventQueue[eventQueueStart - 1] = EVENT_QUEUE_LENGTH - eventQueueStart;
    eventQueue[eventQueueStart - 2] = PL_START_BYTE;

    finishedSendingEvents = false;

    QT_SPI_stopListeningToMaster(); // Stop reading until the data is sent

    QT_SPI_transmit(
            &eventQueue[eventQueueStart - EVENT_QUEUE_HEADER_LENGTH],
            EVENT_QUEUE_LENGTH - eventQueueStart + EVENT_QUEUE_HEADER_LENGTH,
            &OBC,
            flagEventsFinishedSending);

    eventQueueStart = EVENT_QUEUE_LENGTH;
}

byte queryData [4] = { PL_START_BYTE, 2, 0, 0 };

void sendMax() {

}

void sendMin() {

}

void sendDigipot() {

}

void sendTemperature() {
    QT_SPI_stopListeningToMaster();

    queryData[2] = 0xf0;
    queryData[3] = 0x0f;

    QT_SPI_transmit(queryData, 4, &OBC, finishTransmission);
}

void sendSamplingData() {

}

void handleQuery(PL_Query_t query, const byte data [2]) {
    switch(query) {
    case PL_QUERY_EVENT:
        sendEvents();
        break;
    case PL_QUERY_PROBE_DAC_MAX:
        sendMax();
        break;
    case PL_QUERY_PROBE_DAC_MIN:
        sendMin();
        break;
    case PL_QUERY_PROBE_DIGIPOT:
        sendDigipot();
        break;
    case PL_QUERY_PROBE_TEMPERATURE:
        sendTemperature();
        break;
    case PL_QUERY_SAMPLING_DATA:
        sendSamplingData();
        break;
    }
}

void handleCommand(PL_Command_t command) { //, const byte data [2])
    if(commandRunning && (command == PL_COMMAND_POWER_OFF/* || command == PL_COMMAND_STOP_TASK */)) {
        exitCommand = true;
        queueEvent(PL_EVENT_TASK_INTERRUPTED);

        // Wait for the command to halt to ensure that the new value is read by the event loop
        while(commandRunning);
    }

    currentCommand = command;
    commandRunning = true;
}

void obcIncommingHandler(const byte *data) {
    byte code = data[0];

    if(code < PL_COMMAND_ENUM_COUNT) {
        handleCommand((PL_Command_t) code);//, data + 1);
    } else {
        handleQuery((PL_Query_t) code, data + 1);
    }
}

/**
 * Start the SPI communication listening lines.
 */
void startListening() {
    QT_SPI_setReceiveHandler(obcIncommingHandler, 2, &OBC);
    QT_SPI_listenToMaster();
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

static void adcHandler(float data)
{
    volatile float a = data;
}

/*
 * This file handles the communication with the OBC, this is the main event loop that handles
 */

static volatile int pin_flag=1;
void main(void) {
    initialise();

    startListening();

    setupDeploymentTest();
//    startPeriodicTask(DEPLOY_PROBE_SP, 5000, 500);
//    startPeriodicTask(DEPLOY_PROBE_SP, 5000, 500);
    while(true) {
        QT_EADC_measureFloatVoltage(adcHandler);
    }
//    while(true) {
////        QT_EADC_measureSweepCurrent(&adcHandler);
////         Wait until a command has been queued
//        while(!commandRunning);
//
////         These command functions are blocking.
//        switch(currentCommand) {
//        case PL_COMMAND_CALIBRATION_START:
//            queueEvent(PL_EVENT_CALIBRATION_DONE);
//            break;
//        case PL_COMMAND_CALIBRATION_STOP:
//            break;
//            //case PL_COMMAND_DEPLOY:
//            //    break;
//        case PL_COMMAND_POWER_OFF:
//            break;
//        case PL_COMMAND_POWER_ON:
//            queueEvent(PL_EVENT_PROBE_POWERED);
//            break;
//        case PL_COMMAND_SAMPLING_START:
//            break;
//        case PL_COMMAND_SAMPLING_STOP:
//            break;
//        case PL_COMMAND_DEPLOY:
//            QT_BW_deploy();
//            queueEvent(PL_EVENT_PROBE_DEPLOYED);
//        default:
//            break;
//        }
//
//        commandRunning = false;
//    }
}

#pragma vector = PORT2_VECTOR
__interrupt void Port_2(void) {
////    P1OUT ^= BIT1;
//    if (pin_flag ==1) {
//        pin_flag=0;
////        volatile struct timer* timer_item= QT_TIMER_startPeriodicTask(DEPLOY_PROBE_SP, 3000, 100);
//        currentCommand = PL_COMMAND_DEPLOY;
//        handleCommand(PL_COMMAND_DEPLOY);
////        while (timer_item->command != TIMER_STOP) {
////
////        }
////        QT_TIMER_sleep(timer_item, 5000);
////        while (timer_item->command != TIMER_STOP) {
////
////        }
////        timer_item= QT_TIMER_startPeriodicTask(DEPLOY_PROBE_SP, 3000, 100);
//
//        pin_flag=2;
//    } else if (pin_flag == 2) {
//        pin_flag=0;
//
////        currentCommand = PL_COMMAND_ENUM_COUNT;
////        handleCommand(PL_COMMAND_ENUM_COUNT);
////        startPWM(DEPLOY_PROBE_FP, 8000, 10, 80);
//        pin_flag=1;
//    }
//
////    timer_setup(10, 10000);

    P2IFG &= ~BIT0;

}


