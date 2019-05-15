#ifndef BURNWIRE_QT_BW_BURNWIRE_H_
#define BURNWIRE_QT_BW_BURNWIRE_H_

#include "Common/QT_COM_common.h"
#include "Timer/QT_timer.h"
#include "QT_LPMain.h"
#include "InternalADC/QT_adc_internal.h"
#include <stdio.h>
#include <stdbool.h>

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
void QT_BW_deploy();

void QT_BW_toggleSpBurnwire();

void QT_BW_toggleFpBurnwire();

void QT_BW_reset();

#endif /* BURNWIRE_QT_BW_BURNWIRE_H_ */
