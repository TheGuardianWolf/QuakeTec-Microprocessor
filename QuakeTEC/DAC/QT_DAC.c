#include "QT_DAC.h"

#define MAX_SWEEP_VOLTAGE 15.0
#define MIN_SWEEP_VOLTAGE -15.0
#define ZERO_DAC (DAC_VOLTAGE/2)
#define RETRIES_ADC 3

const float SWEEP_SCALE = (1.0/(30.0/DAC_VOLTAGE));

static volatile bool doneTransmitting;
byte DACValue[2] = {0};

//static uint16_t QT_DAC_convertFromDacVoltage(float voltage) {
//    voltage = voltage > DAC_VOLTAGE ? DAC_VOLTAGE : voltage;
//    voltage = voltage < 0 ? 0 : voltage;
//    uint16_t convertedValue = (DAC_RESOLUTION * voltage) / DAC_VOLTAGE;
//    return convertedValue;
//}

static uint16_t QT_DAC_convertFromDacSweepVoltage(float voltage) {
    voltage = voltage > MAX_SWEEP_VOLTAGE ? MAX_SWEEP_VOLTAGE : voltage;
    voltage = voltage < MIN_SWEEP_VOLTAGE ? MIN_SWEEP_VOLTAGE : voltage;
    uint16_t converted_value = (DAC_RESOLUTION / DAC_VOLTAGE) * (ZERO_DAC - (SWEEP_SCALE * voltage));
    return converted_value;
}

bool QT_DAC_setVoltage(float voltage) {
    int i;
    uint8_t status = ERROR_NONE;
    uint16_t convertedValue = QT_DAC_convertFromDacSweepVoltage(voltage);

    for (i=0; i<RETRIES_ADC; i++) {
        status = DAC121S101_Set(convertedValue);
        if (status == ERROR_NONE) {
            break;
        }
    }

    if (status == ERROR_SPI_BUS) {
        ERROR_STATUS |= BIT9;
        queueEvent(PL_EVENT_ERROR);
        return false;
    }

    DACValue[0] = 0xFF00 >> 8;
    DACValue[1] = 0x00FF & convertedValue;
    return true;
}

void QT_DAC_reset() {
    QT_DAC_setVoltage(0);
    QT_DAC_setVoltage(0);
}

byte* QT_DAC_getValue() {
    return DACValue;
}
