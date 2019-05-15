/*
 * QT_timer.h
 *
 *  Created on: 25/02/2019
 *      Author: james
 */

#ifndef TIMER_QT_TIMER_H_
#define TIMER_QT_TIMER_H_

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "driverlib.h"
#include "QT_LPMain.h"

//typedef void (*task_func)();

typedef enum {
    TIMER_STOP,
    DEPLOY_PROBE_SP,
    DEPLOY_PROBE_FP,
    READ_SWEEPING_PROBE,
    READ_FLOATING_PROBE,
    SET_DAC,
    SAMPLE_PROBE,
    TIMER_SLEEP
} timer_command;

struct timer {
    const int id;
    timer_command command;
    uint16_t runtime;
    uint16_t period;
    uint16_t duty_period;
};

extern volatile bool sweepFlag;
extern volatile bool dacFlag;

volatile struct timer* QT_TIMER_startPeriodicTask(timer_command t_command, uint16_t duration, float period);
void QT_TIMER_stopPeriodicTask(volatile struct timer *timer_item);
volatile struct timer* QT_TIMER_startPWM(timer_command t_command, uint16_t duration, float period, uint16_t duty);
volatile struct timer* QT_TIMER_sleep(float period);
int QT_TIMER_timer_sleep(volatile struct timer* timer_item, float period);

void QT_TIMER_resetTaskFlag(timer_command t_command);

#endif /* TIMER_QT_TIMER_H_ */
