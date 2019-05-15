#include <Digipot/QT_DIGIPOT.h>

#define DIGIPOT_MAX_VALUE 49400.0
#define DIGIPOT_RESOLUTION 1024.0
#define DIGIPOT_MAX_RESISTANCE 50000.0

void QT_DIGIPOT_init() {
    AD5292_Init();
    AD5292_SwitchOn();
    QT_DIGIPOT_setGain(2);
}

static uint16_t QT_DIGIPOT_convertGain(float gain) {
    uint16_t u16DigipotValue = DIGIPOT_RESOLUTION * (DIGIPOT_MAX_VALUE / (gain - 1)) / (DIGIPOT_MAX_RESISTANCE);
    return u16DigipotValue;
}

void QT_DIGIPOT_setGain(float gain) {
    uint16_t u16DigipotValue = QT_DIGIPOT_convertGain(gain);
    AD5292_Set(u16DigipotValue);
}
