#include "QT_DAC.h"

extern device_t DAC;

#define MAX_SWEEP_VOLTAGE 15.0
#define MIN_SWEEP_VOLTAGE -15.0
#define ZERO_DAC (DAC_VOLTAGE/2)
const float SWEEP_SCALE = (1.0/(30.0/DAC_VOLTAGE));

static volatile bool doneTransmitting;

static uint16_t QT_DAC_convertToDacVoltage(float voltage) {
    voltage = voltage > DAC_VOLTAGE ? DAC_VOLTAGE : voltage;
    voltage = voltage < 0 ? 0 : voltage;
    uint16_t convertedValue = (DAC_RESOLUTION * voltage) / DAC_VOLTAGE;
    return convertedValue;
}

static uint16_t QT_DAC_convertToDacSweepVoltage(float voltage) {
    voltage = voltage > MAX_SWEEP_VOLTAGE ? MAX_SWEEP_VOLTAGE : voltage;
    voltage = voltage < MIN_SWEEP_VOLTAGE ? MIN_SWEEP_VOLTAGE : voltage;
    uint16_t converted_value = (DAC_RESOLUTION / DAC_VOLTAGE) * (ZERO_DAC - (SWEEP_SCALE * voltage));
    return converted_value;
}

static void QT_DAC_doneTransmitting(bool succeeded){
    doneTransmitting = true;
}

void QT_DAC_setVoltage(float voltage) {
    doneTransmitting = false;
    uint16_t convertedValue = QT_DAC_convertToDacSweepVoltage(voltage);
    byte b1 = (convertedValue & 0xFF00)>>8;
    byte b2 = convertedValue & 0x00FF;
    byte sendData[2] = { b1, b2};
    QT_SPI_transmit(sendData, 2, &DAC, QT_DAC_doneTransmitting);
    __delay_cycles(1000);
    while (!doneTransmitting) {;}

}

void QT_DAC_reset() {
    QT_DAC_setVoltage(0);
}
