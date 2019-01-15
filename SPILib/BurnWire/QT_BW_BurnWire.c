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
static probe_t sweepingProbe = {GPIO_PORT_P3, GPIO_PIN1, GPIO_PORT_P1, GPIO_PIN2};
static probe_t floatingProbe = {GPIO_PORT_P3, GPIO_PIN0, GPIO_PORT_P1, GPIO_PIN1};

/*
 * Private functions
 */

/*
 * Public functions
 */

/**
 * Sets up burn wire module. Should be called before use.
 */
void QT_BW_initialise() {

}

/**
 * Deploys the Langmuir Probe by burning one or both wires.
 */
void QT_BW_deploy() {

}

/*
 * Interrupts
 */
