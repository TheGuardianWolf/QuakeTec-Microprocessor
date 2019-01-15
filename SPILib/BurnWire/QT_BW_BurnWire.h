#ifndef BURNWIRE_QT_BW_BURNWIRE_H_
#define BURNWIRE_QT_BW_BURNWIRE_H_

/*
 * Includes
 */
#include <Common/QT_Common.h>

/*
 * Type definitions
 */

typedef struct {
    uint8_t contactSwitchPort, contactSwitchPin;
    uint8_t burnWirePort, burnWirePin;
} probe_t;

/**
 * Variable declarations
 */

/*
 * Public functions
 */

/**
 * Sets up burn wire module. Should be called before use.
 */
void QT_BW_initialise();

/**
 * Deploys the Langmuir Probe by burning one or both wires.
 */
void QT_BW_deploy();

#endif /* BURNWIRE_QT_BW_BURNWIRE_H_ */
