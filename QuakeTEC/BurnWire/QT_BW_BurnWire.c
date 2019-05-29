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

extern volatile uint16_t ERROR_STATUS;
static byte deploymentRegister[3] = {0};
static byte contactSwitchStatus[1] = {0};

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

//static byte deploymentRegister1 = 0;
//static byte deploymentRegister2 = 0;
//static byte deploymentRegister3 = 0;

/*
 * Private functions
 */

/**
 * Returns true if the probe is in contact with the CubeSat, false otherwise.
 */
static bool QT_BW_isInContact(probe_t *probe) {
    bool status = true;
    int i;
    for (i=0; i<5; i++){
        __delay_cycles(1000);
        status &= GPIO_getInputPinValue(probe->contactSwitchPort, probe->contactSwitchPin);
    }
    return !status;
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
    QT_TIMER_stopPeriodicTask(burn_timer);
    return false; // Probe not deployed
}

static byte QT_BW_burnWireSequence(probe_t *probe, bool useContactSpring){
    uint16_t PWM_duty = probe->duty;
    bool sweepingStatus;
    volatile struct timer* timer_item;
    PWM_duty = probe->duty;
    while ((PWM_duty < MAX_DUTY) && !exitCommand) {
        sweepingStatus = QT_BW_burnWire(probe, useContactSpring);
        //If probe has deployed
        if (sweepingStatus && useContactSpring) {
            return 1;
        }
        PWM_duty += DUTY_INCREMENT;
        probe->duty = PWM_duty;
        timer_item = QT_TIMER_sleep(COOLDOWN_TIME);
        while ((timer_item->command != TIMER_STOP) && (!exitCommand)) {
        }
    }
    return 0;
}

/**
 * Deploys both sweeping and floating probes
 */
static void QT_BW_adaptiveBurn() {
    byte sp_success;
    byte fp_success;
    sp_success = QT_BW_burnWireSequence(&SWEEPING_PROBE, true);
    deploymentRegister[0] |= sp_success;
    deploymentRegister[1] = SWEEPING_PROBE.duty;
    if (sp_success == 0) {
        ERROR_STATUS |= BIT6;
        queueEvent(PL_EVENT_ERROR);
    }
//    ERROR_STATUS |= (1 ^ sp_success) << 6;


    fp_success = QT_BW_burnWireSequence(&FLOATING_PROBE, true);
//    ERROR_STATUS |= (1 ^ fp_success) << 7;
    deploymentRegister[0] |= (fp_success<<1);
    deploymentRegister[2] = FLOATING_PROBE.duty;
    if (fp_success == 0) {
        ERROR_STATUS |= BIT7;
        queueEvent(PL_EVENT_ERROR);
    }
}

/**
 * Deploys both wires, using the given wire's contact status to determine when to stop burning.
 */
static void QT_BW_singleAdaptiveBurn(probe_t *contactProbePtr) {
    byte success;
    success = QT_BW_burnWireSequence(contactProbePtr, true);
    uint16_t best_duty = contactProbePtr->duty >= 85 ? 100 : contactProbePtr->duty + 15;

    if (success == 0) {
        // If not a success, don't try to deploy other probe
        ERROR_STATUS |= (BIT6|BIT7);
        queueEvent(PL_EVENT_ERROR);

    } else if (contactProbePtr == (&SWEEPING_PROBE)) {
        deploymentRegister[1] = SWEEPING_PROBE.duty;
        deploymentRegister[0] |= BIT0;

        FLOATING_PROBE.duty = best_duty;
        QT_BW_burnWire(&FLOATING_PROBE, false);
        // A success
        deploymentRegister[2] = FLOATING_PROBE.duty;
        deploymentRegister[0] |= BIT1;

    } else {
        deploymentRegister[2] = FLOATING_PROBE.duty;
        deploymentRegister[0] |= BIT1;

        SWEEPING_PROBE.duty = best_duty;
        QT_BW_burnWire(&SWEEPING_PROBE, false);
        // A success
        deploymentRegister[1] = SWEEPING_PROBE.duty;
        deploymentRegister[0] |= BIT0;

    }
}

/**
 * Deploys both wires when start state indicates neither is in contact.
 */
static void QT_SW_defaultBurn() {
    SWEEPING_PROBE.duty = DEFAULT_DUTY;
    QT_BW_burnWire(&SWEEPING_PROBE, false);
    deploymentRegister[0] |= BIT0;
    deploymentRegister[1] = SWEEPING_PROBE.duty;

    FLOATING_PROBE.duty = DEFAULT_DUTY;
    QT_BW_burnWire(&FLOATING_PROBE, false);
    deploymentRegister[0] |= BIT1;
    deploymentRegister[2] = FLOATING_PROBE.duty;
}


static void QT_BW_initialise() {

    // Set burn wires as outputs
    GPIO_setAsOutputPin(SWEEPING_PROBE.burnWirePort, SWEEPING_PROBE.burnWirePin);
    GPIO_setAsOutputPin(FLOATING_PROBE.burnWirePort, FLOATING_PROBE.burnWirePin);

    // Set burn wires to off
    GPIO_setOutputLowOnPin(SWEEPING_PROBE.burnWirePort, SWEEPING_PROBE.burnWirePin);
    GPIO_setOutputLowOnPin(FLOATING_PROBE.burnWirePort, FLOATING_PROBE.burnWirePin);

    GPIO_setAsInputPinWithPullUpResistor(SWEEPING_PROBE.contactSwitchPort, SWEEPING_PROBE.contactSwitchPin);
    GPIO_setAsInputPinWithPullUpResistor(FLOATING_PROBE.contactSwitchPort, FLOATING_PROBE.contactSwitchPin);
}

byte* QT_BW_getContactSwitchStatus() {
    bool sp = QT_BW_isInContact(&SWEEPING_PROBE);
    bool fp = QT_BW_isInContact(&FLOATING_PROBE);
    contactSwitchStatus[0] = (byte) sp + ((byte) fp << 1);
    return contactSwitchStatus;
}

byte* QT_BW_getDeploymentStatus() {
    return deploymentRegister;
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
            deploymentRegister[0] |= BIT2;
            deploymentRegister[0] |= BIT3;
            QT_BW_adaptiveBurn();
            // both in contact - adaptive burn for both wires

        } else {
            deploymentRegister[0] |= BIT2;
            QT_BW_singleAdaptiveBurn(&SWEEPING_PROBE); // floating probe not in contact - single adaptive burn
        }
    } else {
        if (QT_BW_isInContact(&FLOATING_PROBE)) {
            deploymentRegister[0] |= BIT3;
            QT_BW_singleAdaptiveBurn(&FLOATING_PROBE); // sweeping probe not in contact - single adaptive burn

        } else {
            QT_SW_defaultBurn(); // neither in contact - default burn
        }
    }
    QT_BW_reset();
    byte* d = QT_BW_getDeploymentStatus();
    int q =1;
    byte a, b, c;
    a = d[0];
    b = d[1];
    c = d[2];
}

void QT_BW_deployFP() {
    QT_BW_initialise();

    if (QT_BW_isInContact(&FLOATING_PROBE)) {
        deploymentRegister[0] |= BIT3;
        QT_BW_singleAdaptiveBurn(&FLOATING_PROBE);
    } else {
        FLOATING_PROBE.duty = DEFAULT_DUTY;
        QT_BW_burnWire(&FLOATING_PROBE, false);
        deploymentRegister[0] |= BIT1;
        deploymentRegister[2] = FLOATING_PROBE.duty;
    }
    QT_BW_reset();
}

void QT_BW_deploySP() {
    QT_BW_initialise();

    if (QT_BW_isInContact(&SWEEPING_PROBE)) {
        deploymentRegister[0] |= BIT2;
        QT_BW_singleAdaptiveBurn(&SWEEPING_PROBE);

    } else {
        SWEEPING_PROBE.duty = DEFAULT_DUTY;
        QT_BW_burnWire(&SWEEPING_PROBE, false);
        deploymentRegister[0] |= BIT0;
        deploymentRegister[1] = SWEEPING_PROBE.duty;
    }
    QT_BW_reset();
}

/*
 * Interrupts
 */
