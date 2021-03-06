#include "QT_EADC.h"

#define RETRIES_ADC 3

extern volatile uint16_t ERROR_STATUS;

void QT_EADC_initialise() {
    ADC128S052_Init();
}

float QT_EADC_convertToVoltage(uint16_t adcValue) {
    float voltage = (float) adcValue * EADC_VOLTAGE / EADC_RESOLUTION;
    return voltage;
}

float QT_EADC_getAdcVoltage(AdcPin adcPin) {
    int i;
    uint16_t data;
    uint8_t status = ERROR_NONE;

    for (i=0; i< RETRIES_ADC; i++) {
        status = ADC128S052_SelectChannel((uint8_t) adcPin);
        if (status == ERROR_NONE) {
            break;
        }
    }
    if (status == ERROR_SPI_BUS) {
        ERROR_STATUS |= BITA;
        queueEvent(PL_EVENT_ERROR);
    }

    for (i=0; i< RETRIES_ADC; i++) {
        status = ADC128S052_Read(&data);
        if (status == ERROR_NONE) {
            break;
        }
    }

    if (status == ERROR_SPI_BUS) {
        ERROR_STATUS |= BITA;
        queueEvent(PL_EVENT_ERROR);
    }

    return QT_EADC_convertToVoltage(data);
}

uint16_t QT_EADC_getAdcValue(AdcPin adcPin) {
    int i;
    uint16_t data;
    uint8_t status = ERROR_NONE;

    for (i=0; i< RETRIES_ADC; i++) {
        status = ADC128S052_SelectChannel((uint8_t) adcPin);
        if (status == ERROR_NONE) {
            break;
        }
    }

    if (status == ERROR_SPI_BUS) {
        ERROR_STATUS |= BITA;
        queueEvent(PL_EVENT_ERROR);
    }

    for (i=0; i< RETRIES_ADC; i++) {
        status = ADC128S052_Read(&data);
        if (status == ERROR_NONE) {
            break;
        }
    }

    if (status == ERROR_SPI_BUS) {
        ERROR_STATUS |= BITA;
        queueEvent(PL_EVENT_ERROR);
    }

    return data;
}
