/*
 * Includes
 */
#include "QT_BW_burnWire.h"

#define TEMP_LIMIT 40.0 // TODO get the temperature limit
#define START_DUTY 30 // TODO get real duty (percentage)
#define START_BURN_LENGTH 3000 // burn duration (ms)
#define START_PWM_PERIOD 10 // duty cycle period (ms)
#define DUTY_INCREMENT 5
#define MAX_DUTY 100
#define NUM_BURN_PROBES 2
#define COOLDOWN_TIME 5000 // time (ms)
#define DEFAULT_DUTY 30

typedef struct {
    uint8_t contactSwitchPort, contactSwitchPin;
    uint8_t burnWirePort, burnWirePin;
    uint16_t duty;
    float burnLength, pwm;
    timer_command command;
    bool status;
} probe_t;

static probe_t SWEEPING_PROBE = {GPIO_PORT_P3, GPIO_PIN1, GPIO_PORT_P1, GPIO_PIN2, START_DUTY, START_BURN_LENGTH, START_PWM_PERIOD, DEPLOY_PROBE_SP, false};
static probe_t FLOATING_PROBE = {GPIO_PORT_P3, GPIO_PIN0, GPIO_PORT_P1, GPIO_PIN1, START_DUTY, START_BURN_LENGTH, START_PWM_PERIOD, DEPLOY_PROBE_FP, false};

/*
 * Private functions
 */

/**
 * Returns true if the probe is in contact with the CubeSat, false otherwise.
 */
static bool QT_BW_isInContact(probe_t *probe) {
    bool status = GPIO_getInputPinValue(probe->contactSwitchPort, probe->contactSwitchPin);
    return !status;
}

/**
 * Retrieves the current temperature from the PCB. Returns true if the temperature is below the limit
 * and it is therefore safe for the probes to be deployed. Returns false otherwise.
 */
static bool QT_BW_temperatureIsSafe() {
    return QT_IADC_readTemperature() < TEMP_LIMIT;
}

/**
 * Burns a wire.
 */
static bool QT_BW_burnWire(probe_t *probe, bool useContactSpring) {
    volatile struct timer* burn_timer = QT_TIMER_startPWM(probe->command, probe->burnLength, START_PWM_PERIOD, probe->duty);
    while (!exitCommand) {
        if (burn_timer->command == TIMER_STOP) {
            break;
        }

        if (!QT_BW_isInContact(probe) && useContactSpring) {
            QT_TIMER_stopPeriodicTask(burn_timer);
            return true; // burn complete
        }
    }
    return false; // Probe not deployed
}

static void QT_BW_burnWireSequence(probe_t *probe, bool useContactSpring){
    uint16_t PWM_duty = probe->duty;
    bool sweepingStatus;
    volatile struct timer* timer_item;
    PWM_duty = probe->duty;
    while ((PWM_duty < MAX_DUTY) && !exitCommand) {
        sweepingStatus = QT_BW_burnWire(probe, useContactSpring);
        //If probe has deployed
        if (sweepingStatus && useContactSpring) {
            break;
        }
        PWM_duty += DUTY_INCREMENT;
        probe->duty = PWM_duty;
        timer_item = QT_TIMER_sleep(COOLDOWN_TIME);
        while ((timer_item->command != TIMER_STOP) && (!exitCommand)) {
        }
    }
}

/**
 * Deploys both sweeping and floating probes
 */
static void QT_BW_adaptiveBurn() {
    QT_BW_burnWireSequence(&SWEEPING_PROBE, true);
    QT_BW_burnWireSequence(&FLOATING_PROBE, true);
}

/**
 * Deploys both wires, using the given wire's contact status to determine when to stop burning.
 */
static void QT_BW_singleAdaptiveBurn(probe_t *contactProbePtr) {
    QT_BW_burnWireSequence(contactProbePtr, true);
    uint16_t best_duty = contactProbePtr->duty >= 85 ? 100 : contactProbePtr->duty + 15;
    if (contactProbePtr == (&SWEEPING_PROBE)) {
        FLOATING_PROBE.duty = best_duty;
        QT_BW_burnWire(&FLOATING_PROBE, false);
    } else {
        SWEEPING_PROBE.duty = best_duty;
        QT_BW_burnWire(&SWEEPING_PROBE, false);
    }
}

/**
 * Deploys both wires when start state indicates neither is in contact.
 */
static void QT_SW_defaultBurn() {
    SWEEPING_PROBE.duty = DEFAULT_DUTY;
    FLOATING_PROBE.duty = DEFAULT_DUTY;
    QT_BW_burnWire(&SWEEPING_PROBE, false);
    QT_BW_burnWire(&FLOATING_PROBE, false);
}


static void QT_BW_initialise() {

    // Set burn wires as outputs
    GPIO_setAsOutputPin(SWEEPING_PROBE.burnWirePort, SWEEPING_PROBE.burnWirePin);
    GPIO_setAsOutputPin(FLOATING_PROBE.burnWirePort, FLOATING_PROBE.burnWirePin);

    // Set burn wires to off
    GPIO_setOutputLowOnPin(SWEEPING_PROBE.burnWirePort, SWEEPING_PROBE.burnWirePin);
    GPIO_setOutputLowOnPin(FLOATING_PROBE.burnWirePort, FLOATING_PROBE.burnWirePin);

//    P3OUT &= ~(BIT0|BIT1);
//    P3DIR &= ~(BIT0|BIT1);
//    P3REN |= BIT0|BIT1;
    GPIO_setAsInputPinWithPullUpResistor(SWEEPING_PROBE.contactSwitchPort, SWEEPING_PROBE.contactSwitchPin);
    GPIO_setAsInputPinWithPullUpResistor(FLOATING_PROBE.contactSwitchPort, FLOATING_PROBE.contactSwitchPin);
}

void QT_BW_reset() {
    GPIO_setAsOutputPin(SWEEPING_PROBE.burnWirePort, SWEEPING_PROBE.burnWirePin);
    GPIO_setAsOutputPin(FLOATING_PROBE.burnWirePort, FLOATING_PROBE.burnWirePin);
    GPIO_setOutputLowOnPin(SWEEPING_PROBE.burnWirePort, SWEEPING_PROBE.burnWirePin);
    GPIO_setOutputLowOnPin(FLOATING_PROBE.burnWirePort, FLOATING_PROBE.burnWirePin);
    GPIO_setAsInputPinWithPullDownResistor(SWEEPING_PROBE.contactSwitchPort, SWEEPING_PROBE.contactSwitchPin);
    GPIO_setAsInputPinWithPullDownResistor(FLOATING_PROBE.contactSwitchPort, FLOATING_PROBE.contactSwitchPin);
}

void QT_BW_toggleSpBurnwire() {
    SWEEPING_PROBE.status  = !SWEEPING_PROBE.status;
    GPIO_setAsOutputPin(SWEEPING_PROBE.burnWirePort, SWEEPING_PROBE.burnWirePin);
    GPIO_toggleOutputOnPin(SWEEPING_PROBE.burnWirePort, SWEEPING_PROBE.burnWirePin);
}

void QT_BW_toggleFpBurnwire() {
    FLOATING_PROBE.status  = !FLOATING_PROBE.status;
    GPIO_setAsOutputPin(FLOATING_PROBE.burnWirePort, FLOATING_PROBE.burnWirePin);
    GPIO_toggleOutputOnPin(FLOATING_PROBE.burnWirePort, FLOATING_PROBE.burnWirePin);
}

void QT_BW_deploy() {
    QT_BW_initialise();

    if (QT_BW_isInContact(&SWEEPING_PROBE)) {
        if (QT_BW_isInContact(&FLOATING_PROBE)) {
            QT_BW_adaptiveBurn();
            // both in contact - adaptive burn for both wires

        } else {
            QT_BW_singleAdaptiveBurn(&SWEEPING_PROBE); // floating probe not in contact - single adaptive burn
        }
    } else {
        if (QT_BW_isInContact(&FLOATING_PROBE)) {

            QT_BW_singleAdaptiveBurn(&FLOATING_PROBE); // sweeping probe not in contact - single adaptive burn

        } else {
            QT_SW_defaultBurn(); // neither in contact - default burn
        }
    }
    QT_BW_reset();
}

/*
 * Interrupts
 */
