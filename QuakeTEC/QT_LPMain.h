/*
 * QT_LPMain.h
 *
 *  Created on: 20/02/2019
 *      Author: james
 */

#ifndef QT_LPMAIN_H_
#define QT_LPMAIN_H_

#include "driverlib.h"

#include "InternalADC/QT_adc_internal.h"
#include "ExternalADC/QT_EADC.h"
#include "SpiLib/QT_SPI_Protocol.h"
#include "SpiLib/QT_SPI_SpiLib.h"
#include "BurnWire/QT_BW_BurnWire.h"
#include "Sweep/QT_SW_sweep.h"
#include "Timer/QT_timer.h"
#include "PowerControl/QT_PWR_power.h"

typedef struct //function_flags
{
    uint16_t probe_deploying;
} f_flags;

extern volatile f_flags F_FLAGS;

/** Set to true if a command should exit as soon as possible. Commands must not reset this flag, and should leave their systems in a 'safe' state. */
extern volatile bool exitCommand;

#endif /* QT_LPMAIN_H_ */
