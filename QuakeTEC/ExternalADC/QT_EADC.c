#include "QT_EADC.h"

extern device_t ADC;

static volatile uint16_t adcData;
static volatile bool doneReceiving;
static volatile bool doneTransmitting;

static void QT_EADC_handler(const byte *data) {
    adcData = (data[0]<<8) + data[1];
    uint16_t a = adcData;
    doneReceiving = true;
}

static void QT_EADC_doneTransmitting(bool succeeded){
    doneTransmitting = true;
}

void QT_EADC_initialise() {
    QT_SPI_setReceiveHandler(QT_EADC_handler, 2, &ADC);
    QT_DAC_setVoltage(0);
}

float QT_EADC_getAdcVoltage() {
    uint16_t result = QT_EADC_getAdcValue();
    float voltage = (float) result * EADC_VOLTAGE / EADC_RESOLUTION;
    return voltage;
}

uint16_t QT_EADC_getAdcValue() {
    while (!doneReceiving) {;}
    uint16_t data = adcData;
    return adcData;
}

bool QT_EADC_adcRead(AdcPin adcPin) {
    doneReceiving = false;
    doneTransmitting = false;
    uint16_t data = adcData;
    byte sendData[2] = {adcPin<<3 , 0x00};
    ADC.receiveHandler = QT_EADC_handler;
    QT_SPI_transmit(sendData, 2, &ADC, QT_EADC_doneTransmitting);
    __delay_cycles(1000);
//    while (!doneTransmitting) {;}
    return true;
}


