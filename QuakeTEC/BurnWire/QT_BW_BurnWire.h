#ifndef BURNWIRE_QT_BW_BURNWIRE_H_
#define BURNWIRE_QT_BW_BURNWIRE_H_

#include <stdio.h>
#include <stdbool.h>
#include "Common/QT_COM_common.h"
#include "Timer/QT_timer.h"
#include "OBCInterface/QT_OBC_Interface.h"
#include "InternalADC/QT_adc_internal.h"
#include "PL_Protocol.h"

/*
 * Includes
 */


/*
 * Type definitions
 */


/**
 * Variable declarations
 */

/*
 * Public functions
 */

/**
 * Sets up burn wire module. Should be called before use.
 */

/**
 * Deploys the Langmuir Probe by burning one or both wires.
 */

byte* QT_BW_getContactSwitchStatus();

byte* QT_BW_getDeploymentStatus();

void QT_BW_deploy();

void QT_BW_toggleSpBurnwire();

void QT_BW_toggleFpBurnwire();

void QT_BW_reset();

void QT_BW_deployFP();

void QT_BW_deploySP();



#endif /* BURNWIRE_QT_BW_BURNWIRE_H_ */
