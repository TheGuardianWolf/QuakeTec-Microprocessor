/*
 * Includes
 */
#include <InternalADC/QT_adc_internal.h>
#include "BurnWire/QT_BW_BurnWire.h"
#include "Common/QT_COM_common.h"
#include "Timer/QT_timer.h"
#include "QT_LPMain.h"

/*
 * Defines
 */
#define TEMP_LIMIT 40.0 // TODO get the temperature limit
#define START_DUTY 30 // TODO get real duty (percentage)
#define START_BURN_LENGTH 3000 // burn duration (ms)
#define START_PWM_PERIOD 10 // duty cycle period (ms)
#define DUTY_INCREMENT 5
#define MAX_DUTY 100
#define NUM_BURN_PROBES 2
#define COOLDOWN_TIME 5000 // time (ms)
#define DEFAULT_DUTY 30

/*
 * Private variables
 */

/**
 * Sweeping probe or floating probe
 */
typedef struct {
    uint8_t contactSwitchPort, contactSwitchPin;
    uint8_t burnWirePort, burnWirePin;
    uint16_t duty;
    float burnLength, pwm;
    timer_command command;
} probe_t;

static probe_t SWEEPING_PROBE = {GPIO_PORT_P3, GPIO_PIN1, GPIO_PORT_P1, GPIO_PIN2, START_DUTY, START_BURN_LENGTH, START_PWM_PERIOD, DEPLOY_PROBE_SP};
static probe_t FLOATING_PROBE = {GPIO_PORT_P3, GPIO_PIN0, GPIO_PORT_P1, GPIO_PIN1, START_DUTY, START_BURN_LENGTH, START_PWM_PERIOD, DEPLOY_PROBE_FP};

/*
 * Private functions
 */

/**
 * Returns true if the probe is in contact with the CubeSat, false otherwise.
 */
static bool isInContact(probe_t *probe) {
    bool status = GPIO_getInputPinValue(probe->contactSwitchPort, probe->contactSwitchPin);
    return status;
}

/**
 * Retrieves the current temperature from the PCB. Returns true if the temperature is below the limit
 * and it is therefore safe for the probes to be deployed. Returns false otherwise.
 */
static bool temperatureIsSafe() {
    return QT_IADC_readTemperature() < TEMP_LIMIT;
}

/**
 * Burns a wire.
 */
static bool burnWire(probe_t *probe, bool useContactSpring) {
    volatile struct timer* burn_timer = QT_TIMER_startPWM(probe->command, probe->burnLength, START_PWM_PERIOD, probe->duty);
    while (!exitCommand) {
        if (burn_timer->command == TIMER_STOP) {
            break;
        }

        if (!isInContact(probe) && useContactSpring) {
            QT_TIMER_stopPeriodicTask(burn_timer);
            return true; // burn complete
        }
    }
    P1OUT &= ~BIT0;
    return false; // Probe not deployed
}

static void burnWireSequence(probe_t *probe, bool useContactSpring){
    uint16_t PWM_duty = probe->duty;
    bool sweepingStatus;
    volatile struct timer* timer_item;
    PWM_duty = probe->duty;
    while ((PWM_duty < MAX_DUTY) && !exitCommand) {
        sweepingStatus = burnWire(probe, useContactSpring);
        //If probe has deployed
        if (sweepingStatus && useContactSpring) {
            break;
        }
        PWM_duty += DUTY_INCREMENT;
        probe->duty = PWM_duty;
        timer_item = QT_sleep(COOLDOWN_TIME);
        while ((timer_item->command != TIMER_STOP) && (!exitCommand)) {
        }
    }
}

/**
 * Deploys both sweeping and floating probes
 */
static void adaptiveBurn() {
    burnWireSequence(&SWEEPING_PROBE, true);
    burnWireSequence(&FLOATING_PROBE, true);
}

/**
 * Deploys both wires, using the given wire's contact status to determine when to stop burning.
 */
static void singleAdaptiveBurn(probe_t *contactProbePtr) {
    burnWireSequence(contactProbePtr, true);
    uint16_t best_duty = contactProbePtr->duty >= 85 ? 100 : contactProbePtr->duty + 15;
    if (contactProbePtr == (&SWEEPING_PROBE)) {
        FLOATING_PROBE.duty = best_duty;
        burnWire(&FLOATING_PROBE, false);
    } else {
        SWEEPING_PROBE.duty = best_duty;
        burnWire(&SWEEPING_PROBE, false);
    }
}

/**
 * Deploys both wires when start state indicates neither is in contact.
 */
static void defaultBurn() {
    SWEEPING_PROBE.duty = DEFAULT_DUTY;
    FLOATING_PROBE.duty = DEFAULT_DUTY;
    burnWire(&SWEEPING_PROBE, false);
    burnWire(&FLOATING_PROBE, false);
}

/*
 * Public functions
 */

/**
 * Sets up burn wire module. Should be called before use.
 */
void QT_BW_initialise() {


    // Set contact switches as inputs
    GPIO_setAsInputPin(SWEEPING_PROBE.contactSwitchPort, SWEEPING_PROBE.contactSwitchPin);
    GPIO_setAsInputPin(FLOATING_PROBE.contactSwitchPort, FLOATING_PROBE.contactSwitchPin);

    // TODO Setup temperature sensor - use James's module?

    // Set burn wires as outputs
    GPIO_setAsOutputPin(SWEEPING_PROBE.burnWirePort, SWEEPING_PROBE.burnWirePin);
    GPIO_setAsOutputPin(FLOATING_PROBE.burnWirePort, FLOATING_PROBE.burnWirePin);

    // Set burn wires to off
    GPIO_setOutputLowOnPin(SWEEPING_PROBE.burnWirePort, SWEEPING_PROBE.burnWirePin);
    GPIO_setOutputLowOnPin(FLOATING_PROBE.burnWirePort, FLOATING_PROBE.burnWirePin);

//    SWEEPING_PROBE = {GPIO_PORT_P3, GPIO_PIN1, GPIO_PORT_P1, GPIO_PIN2, START_DUTY, START_BURN_LENGTH, START_PWM_PERIOD, DEPLOY_PROBE_SP};
//    FLOATING_PROBE = {GPIO_PORT_P3, GPIO_PIN0, GPIO_PORT_P1, GPIO_PIN1, START_DUTY, START_BURN_LENGTH, START_PWM_PERIOD, DEPLOY_PROBE_FP};
    P3OUT &= ~(BIT0|BIT1);
    P3DIR &= ~(BIT0|BIT1);
    P3REN |= BIT0|BIT1;
}

/**
 * Deploys the Langmuir Probe by burning the wires.
 */
void QT_BW_deploy() {
    QT_BW_initialise();

    if (isInContact(&SWEEPING_PROBE)) {
        if (isInContact(&FLOATING_PROBE)) {
            adaptiveBurn();
            // both in contact - adaptive burn for both wires

        } else {
            singleAdaptiveBurn(&SWEEPING_PROBE); // floating probe not in contact - single adaptive burn
        }
    } else {
        if (isInContact(&FLOATING_PROBE)) {

            singleAdaptiveBurn(&FLOATING_PROBE); // sweeping probe not in contact - single adaptive burn

        } else {
            defaultBurn(); // neither in contact - default burn
        }
    }

}

/*
 * Interrupts
 */
