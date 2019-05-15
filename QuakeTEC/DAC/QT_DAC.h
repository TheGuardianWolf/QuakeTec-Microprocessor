#ifndef QT_DAC_H_
#define QT_DAC_H_

#include "QT_DAC121S101.h"

#define DAC_RESOLUTION 4095.0
#define DAC_VOLTAGE 3.3

void QT_DAC_setVoltage(float value);
void QT_DAC_reset();

#endif /* QT_DAC_H_ */
