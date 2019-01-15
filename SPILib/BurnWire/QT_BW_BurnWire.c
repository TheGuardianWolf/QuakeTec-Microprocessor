/*
 * Includes
 */
#include <BurnWire/QT_BW_BurnWire.h>

/*
 * Defines
 */

/*
 * Private variables
 */
static probe_t SWEEPING_PROBE = {GPIO_PORT_P3, GPIO_PIN1, GPIO_PORT_P1, GPIO_PIN2};
static probe_t FLOATING_PROBE = {GPIO_PORT_P3, GPIO_PIN0, GPIO_PORT_P1, GPIO_PIN1};

/*
 * Private functions
 */

/**
 * Returns true if the probe is in contact with the CubeSat, false otherwise.
 */
static bool isInContact(probe_t *probe) {
    return GPIO_getInputPinValue(probe->contactSwitchPort, probe->contactSwitchPin);
}

/*
 * Public functions
 */

/**
 * Sets up burn wire module. Should be called before use.
 */
void QT_BW_initialise() {

    // Set contact switches as inputs
    GPIO_setAsOutputPin(SWEEPING_PROBE.contactSwitchPort, SWEEPING_PROBE.contactSwitchPin);
    GPIO_setAsOutputPin(FLOATING_PROBE.contactSwitchPort, FLOATING_PROBE.contactSwitchPin);

    // Set burn wires as outputs
    GPIO_setAsOutputPin(SWEEPING_PROBE.burnWirePort, SWEEPING_PROBE.burnWirePin);
    GPIO_setAsOutputPin(FLOATING_PROBE.burnWirePort, FLOATING_PROBE.burnWirePin);

    // Set burn wires to off
    GPIO_setOutputLowOnPin(SWEEPING_PROBE.burnWirePort, SWEEPING_PROBE.burnWirePin);
    GPIO_setOutputLowOnPin(FLOATING_PROBE.burnWirePort, FLOATING_PROBE.burnWirePin);
}

/**
 * Deploys the Langmuir Probe by burning the wires.
 */
void QT_BW_deploy() {

    // TODO general housekeeping and checking states

    if (isInContact(&SWEEPING_PROBE)) {
        if (isInContact(&FLOATING_PROBE)) {
            // both in contact - adaptive burn
        } else {
            // floating probe not in contact - single adaptive burn - wait for floating probe to deploy
        }
    } else {
        if (isInContact(&FLOATING_PROBE)) {
            // sweeping probe not in contact - single adaptive burn - wait for sweeping probe to deploy
        } else {
            // neither in contact - default burn
        }
    }

    // TODO general housekeeping and checking states
}

/*
 * Interrupts
 */
