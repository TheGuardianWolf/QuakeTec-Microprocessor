#include <Digipot/QT_DIGIPOT.h>

#define DIGIPOT_MAX_VALUE 49400.0
#define DIGIPOT_RESOLUTION 1024.0
#define DIGIPOT_MAX_RESISTANCE 50000.0

extern volatile uint16_t ERROR_STATUS;

static byte value[2]= {0,0};

void QT_DIGIPOT_init() {
    AD5292_Init();
    AD5292_SwitchOn();
    QT_DIGIPOT_setGain(2);
}

static uint16_t QT_DIGIPOT_convertGain(float gain) {
    uint16_t u16DigipotValue = DIGIPOT_RESOLUTION * (DIGIPOT_MAX_VALUE / (gain - 1)) / (DIGIPOT_MAX_RESISTANCE);
    return u16DigipotValue;
}

bool QT_DIGIPOT_setGain(float gain) {
    uint8_t status = ERROR_NONE;
    uint16_t u16DigipotValue = QT_DIGIPOT_convertGain(gain);
    status = AD5292_Set(u16DigipotValue);
    if (status == ERROR_SPI_BUS) {
        ERROR_STATUS |= BITB;
        queueEvent(PL_EVENT_ERROR);
        return false;
    }
    __delay_cycles(1000000);
    return true;
}

byte* QT_DIGIPOT_getValue() {
    uint16_t digipotValue;
    uint8_t status = ERROR_NONE;
    status = AD5292_Get(&digipotValue);
    if (status == ERROR_SPI_BUS) {
        ERROR_STATUS |= BITB;
        queueEvent(PL_EVENT_ERROR);
        return value;
    }
    value[0] = digipotValue >> 8;
    value[1] = digipotValue & 0x00FF;
    return value;
}
