/*
 * QT_timer.h
 *
 *  Created on: 25/02/2019
 *      Author: james
 */

#ifndef TIMER_QT_TIMER_H_
#define TIMER_QT_TIMER_H_

#include <stdint.h>

typedef void (*task_func)();

void QT_TIMER_startPWM(float duty, task_func turnOn, task_func turnOff);
void QT_TIMER_stopPWM();

void QT_TIMER_sleep(uint16_t micros);

void QT_TIMER_startPeriodicTask(task_func task, uint16_t micros);
void QT_TIMER_stopPeriodicTask();

#endif /* TIMER_QT_TIMER_H_ */
