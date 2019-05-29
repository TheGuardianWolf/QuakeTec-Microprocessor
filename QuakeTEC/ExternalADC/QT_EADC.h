#ifndef EXTERNALADC_QT_ADC_EXTERNAL_H_
#define EXTERNALADC_QT_ADC_EXTERNAL_H_

#include <stdbool.h>
#include "QT_ADC128S052.h"
#include "Common/QT_COM_common.h"
#include "driverlib.h"

#define EADC_RESOLUTION 4095.0
#define EADC_VOLTAGE 3.3
#define EADC_MIN_VALUE 150.0
#define EADC_MAX_VALUE 3950.0

typedef enum {
    ADC0,
    ADC1,
    ADC2,
    ADC3,
    ADC4,
    ADC5,
    ADC6,
    ADC7,
} AdcPin;

//bool QT_EADC_adcRead(AdcPin adcPin);

uint16_t QT_EADC_getAdcValue(AdcPin adcPin);

float QT_EADC_getAdcVoltage();

void QT_EADC_initialise();

#endif
