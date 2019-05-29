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

typedef struct //function_flags
{
    uint16_t probe_deploying;
} f_flags;

extern volatile f_flags F_FLAGS;


/** Set to true if a command should exit as soon as possible. Commands must not reset this flag, and should leave their systems in a 'safe' state. */
volatile PL_Command_t currentCommand;
volatile bool commandRunning;

void startListening();
void obcIncomingHandler(const byte *data);
void queueEvent(PL_Event_t event);

#endif
