#ifndef QT_OBC_INTERFACE_H_
#define QT_OBC_INTERFACE_H_

#include "driverlib.h"
#include "SpiLib/QT_SPI_SpiLib.h"
#include "PL_Protocol.h"
#include "Common/QT_COM_common.h"
#include "BurnWire/QT_BW_BurnWire.h"
#include "DAC/QT_DAC.h"
#include "Digipot/QT_DIGIPOT.h"
#include "Sweep/QT_SW_sweep.h"
#include "InternalADC/QT_adc_internal.h"


void QT_OBC_Interface_init();

void queueEvent(PL_Event_t event);

void QT_OBC_Interface_commandLoop();

#endif
