#include "QT_PWR_power.h"

#define POWER_16V_PORT 3
#define POWER_16V_PIN GPIO_PIN2
#define POWER_16V_DBG_PORT 3
#define POWER_16V_DBG_PIN GPIO_PIN4
#define POWER_16V_CUR_SENSE_PORT 5
#define POWER_16V_CUR_SENSE_PIN GPIO_PIN0
#define POWER_GUARD_PORT 3
#define POWER_GUARD_PIN GPIO_PIN3

void QT_PWR_turnOn16V( ) {
    GPIO_setAsOutputPin(POWER_16V_PORT, POWER_16V_PIN);
    GPIO_setOutputHighOnPin(POWER_16V_PORT, POWER_16V_PIN);
    int a =1;
}

void QT_PWR_turnOff16V() {
    GPIO_setAsOutputPin(POWER_16V_PORT, POWER_16V_PIN);
    GPIO_setOutputLowOnPin(POWER_16V_PORT, POWER_16V_PIN);
}

void QT_PWR_turnOnGuard() {
    GPIO_setAsOutputPin(POWER_GUARD_PORT, POWER_GUARD_PIN);
    GPIO_setOutputHighOnPin(POWER_GUARD_PORT, POWER_GUARD_PIN);
}

void QT_PWR_turnOffGuard() {
    GPIO_setAsOutputPin(POWER_GUARD_PORT, POWER_GUARD_PIN);
    GPIO_setOutputLowOnPin(POWER_GUARD_PORT, POWER_GUARD_PIN);
}

float QT_PWR_getCurrent16V() {
    //TODO read in adc value at cur sense pin
    return 0.0;
}

//typedef struct {
//    uint16_t port;
//    uint16_t pin;
//    uint16_t debugPort;
//    uint16_t debugPin;
//    uint16_t currentPort;
//    uint16_t currentPin;
//    bool status;
//} power_controller;
//
//static power_controller power16V = {3, 2,
