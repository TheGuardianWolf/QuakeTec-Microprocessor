#include <stdlib.h>

#include "QT_timer.h"
#include "driverlib.h"

static volatile task_func taskTurnOn = NULL;
static volatile task_func taskTurnOff = NULL;

static volatile task_func periodicTask = NULL;

static volatile uint16_t overflowsRemaining = 0;

/*
 * CCR1 is used to turn on the PWM line
 * CCR2 is used to wake from sleep
 * Timer reset is used to turn off the PWM line
 */

// The timer length in clock cycles
#define TIMER_LENGTH 10000

void QT_TIMER_initialise() {
    //Start timer
    Timer_B_initUpModeParam param = {0};
    param.clockSource = TIMER_B_CLOCKSOURCE_SMCLK;
    param.clockSourceDivider = TIMER_B_CLOCKSOURCE_DIVIDER_1;
    param.timerPeriod = 11500;
    param.timerInterruptEnable_TBIE = TIMER_B_TBIE_INTERRUPT_ENABLE;
    param.captureCompareInterruptEnable_CCR0_CCIE = TIMER_B_CCIE_CCR0_INTERRUPT_DISABLE;
    param.timerClear = TIMER_B_DO_CLEAR;
    param.startTimer = false;
    //Timer_B_initUpMode(TIMER_B0_BASE, &param);

    Timer_B_initUpModeParam paramTimerB1 = {0};
    paramTimerB1.clockSource = TIMER_B_CLOCKSOURCE_SMCLK;
    paramTimerB1.clockSourceDivider = TIMER_B_CLOCKSOURCE_DIVIDER_1;
    paramTimerB1.timerPeriod = 5000;
    paramTimerB1.timerInterruptEnable_TBIE = TIMER_B_TBIE_INTERRUPT_DISABLE;
    paramTimerB1.captureCompareInterruptEnable_CCR0_CCIE = TIMER_B_CCIE_CCR0_INTERRUPT_DISABLE;
    paramTimerB1.timerClear = TIMER_B_DO_CLEAR;
    paramTimerB1.startTimer = true;
    Timer_B_initUpMode(TIMER_B1_BASE, &paramTimerB1);

    //Initialize compare mode to generate PWM1
    Timer_B_initCompareModeParam param1 = {0};
    param1.compareRegister = TIMER_B_CAPTURECOMPARE_REGISTER_1;
    param1.compareInterruptEnable = TIMER_B_CAPTURECOMPARE_INTERRUPT_ENABLE;
    param1.compareOutputMode = TIMER_B_OUTPUTMODE_OUTBITVALUE;
    param1.compareValue = 0;
    //Timer_B_initCompareMode(TIMER_B0_BASE, &param1);

    param1.compareRegister = TIMER_B_CAPTURECOMPARE_REGISTER_2;
    param1.compareInterruptEnable = TIMER_B_CAPTURECOMPARE_INTERRUPT_ENABLE;
    param1.compareOutputMode = TIMER_B_OUTPUTMODE_OUTBITVALUE;
    param1.compareValue = 0;
    //Timer_B_initCompareMode(TIMER_B0_BASE, &param1);

    //Timer_B_startCounter(TIMER_B0_BASE, TIMER_B_UP_MODE);
}

void QT_TIMER_startPWM(float duty, task_func turnOn, task_func turnOff) {
    taskTurnOn = turnOn;
    taskTurnOff = turnOff;

    Timer_B_setCompareValue(TIMER_B0_BASE, TIMER_B_CAPTURECOMPARE_REGISTER_0, (uint16_t) (TIMER_LENGTH * (1.0 - duty)));
}

void QT_TIMER_stopPWM() {
    taskTurnOn = NULL;
    (*taskTurnOff)();
    taskTurnOff = NULL;
}

void QT_TIMER_sleep(uint16_t micros) {
    // Set the CCR1 and hit it X times.

    uint64_t clocks = micros * CS_getMCLK() / 1000000;

    uint16_t compare = Timer_B_getCaptureCompareCount(TIMER_B0_BASE, TIMER_B_CAPTURECOMPARE_REGISTER_1);

    Timer_B_setCompareValue(TIMER_B0_BASE, TIMER_B_CAPTURECOMPARE_REGISTER_1, (uint16_t) ((compare + clocks) % TIMER_LENGTH));
    overflowsRemaining = clocks / TIMER_LENGTH;

    while(overflowsRemaining != 0) {
        __bic_SR_register(LPM0_bits | GIE);
    }
}

void QT_TIMER_startPeriodicTask(task_func task, uint16_t micros) {
    // uint16_t clocks = (uint16_t) (micros * CS_getMCLK() / 1000000);

    // Timer_B_selectCounterLength(TIMER_B1_BASE, clocks);
    // Timer_B_clear(TIMER_B1_BASE);
    periodicTask = task;
}

void QT_TIMER_stopPeriodicTask() {
    periodicTask = NULL;
}

#pragma vector=TIMERB0_VECTOR
__interrupt
void TIMERB0_ISR (void)
{
    switch(TB0IV) {
    case TB0IV_TB0CCR1:
        // Compare register, trigger PWM on
        if(taskTurnOn != NULL) {
            (*taskTurnOn)();
        }

        break;
    case TB0IV_TBIFG:
        // Timer overflow, trigger timer overflow, trigger PWM off
        if(taskTurnOff != NULL) {
            (*taskTurnOff)();
        }

        if(overflowsRemaining > 0) {
            overflowsRemaining--;
        }

        break;
    case TB0IV_TB0CCR2:
        // Compare 2, wake from LPM0
        if(overflowsRemaining == 0) {
            __bic_SR_register_on_exit(LPM0_bits);
        }
    }
}

#pragma vector=TIMERB1_VECTOR
__interrupt
void TIMERB1_ISR (void)
{
    // Periodic task
    if(periodicTask != NULL) {
        (*periodicTask)();
    }
}
