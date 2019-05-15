#ifndef QT_PWR_POWER_H_
#define QT_PWR_POWER_H_

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "driverlib.h"
#include "Timer/QT_timer.h"
#include "InternalADC/QT_adc_internal.h"

void QT_PWR_turnOn16V();

void QT_PWR_turnOff16V();

void QT_PWR_turnOnGuard();

void QT_PWR_turnOffGuard();

float QT_PWR_getCurrent16V();

#endif
