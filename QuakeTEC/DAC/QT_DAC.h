#ifndef QT_DAC_H_
#define QT_DAC_H_

#include <stdbool.h>
#include "QT_DAC121S101.h"
#include "Common/QT_COM_common.h"
#include "PL_Protocol.h"
#include "OBCInterface/QT_OBC_Interface.h"

#define DAC_RESOLUTION 4095.0
#define DAC_VOLTAGE 5.0

bool QT_DAC_setVoltage(float value);
void QT_DAC_reset();
byte* QT_DAC_getValue();

#endif /* QT_DAC_H_ */
