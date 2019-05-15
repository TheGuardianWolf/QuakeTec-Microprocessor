
#include "QT_timer.h"
#include <stdio.h>

volatile bool sweepFlag = false;
volatile bool dacFlag = false;

//Either smclk or aclk
const float clock_select[2] = {0.00025, 0.1};
const int ID_select[4] = {1, 2, 4, 8};
const int TBIDEX_select[8] = {1, 2, 3, 4, 5, 6, 7, 8};

static volatile struct timer timer0 = {0, TIMER_STOP, 0, 0, 0};
static volatile struct timer timer1 = {1, TIMER_STOP, 0, 0, 0};
static volatile struct timer timer2 = {2, TIMER_STOP, 0, 0, 0};
static volatile struct timer timer3 = {3, TIMER_STOP, 0, 0, 0};

static void QT_TIMER_initialise(volatile struct timer *timer_item, unsigned int ctl_reg, unsigned int tbx_reg, float ticks) { //, int CTL_register, int TBI_register
    int timer_no = timer_item->id;
    uint16_t period = (uint16_t) ticks;
    switch(timer_no) {
    case 0:
        timer0.period = period;
        TB0CCR0 = timer0.period;
        TB0CCTL0 = CCIE | OUTMOD_5;
        TB0CTL = (ctl_reg | MC_1);
        TB0EX0 = tbx_reg;
        break;
    case 1:
        timer1.period = period;
        TB1CCR0 = timer1.period;
        TB1CCTL0 = CCIE | OUTMOD_5;
        TB1CTL = (ctl_reg | MC_1);
        TB1EX0 = tbx_reg;
        break;
    case 2:
        timer2.period = period;
        TB2CCR0 = timer2.period;
        TB2CCTL0 = CCIE | OUTMOD_5;
        TB2CTL = (ctl_reg | MC_1);
        TB2EX0 = tbx_reg;
        break;
    case 3:
        timer3.period = period;
        TB3CCR0 = timer3.period;
        TB3CCTL0 = CCIE | OUTMOD_5;
        TB3CTL = (ctl_reg | MC_1);
        TB3EX0 = tbx_reg;
        break;
    }
    __bis_SR_register(GIE);
//    __no_operation();
}

static uint16_t QT_TIMER_setup(volatile struct timer *timer_item, uint16_t duration, float period) {
//    volatile uint32_t smclk_speed = CS_getSMCLK();
//    clock_select[0] = 1000/smclk_speed;
    float compare_value=0;
    float period_ticks;
    int i, j, k;
    unsigned int timing_control = 0;
    unsigned int tbx_control = 0;

    for (i = 0; i<2; i++) {
        for (j = 0; j<4; j++) {
            for (k = 0; k<8; k++) {
                compare_value = (clock_select[i]*ID_select[j]*TBIDEX_select[k]*65536);
                if (period < compare_value) {
                    break;
                }
            }
            if (period < (compare_value)) {
                break;
            }
        }
        if (period < (compare_value)) {
            break;
        }
    }

    if (compare_value < period) {
        return NULL;
    }

    if (i==0) {
        timing_control |= BIT9;
    } else {
        timing_control |= BIT8;
    }

    timing_control |= (j<<6);
    tbx_control = k;
    period_ticks = period/compare_value;
    period_ticks = period_ticks * 65536;

    QT_TIMER_initialise(timer_item, timing_control, tbx_control, period_ticks);

    return 1;
}

volatile struct timer* QT_TIMER_sleep(float period) {
    uint16_t duration = 0;
    timer_command t_command = TIMER_SLEEP;
    return QT_TIMER_startPeriodicTask(t_command, duration, period);
}

int QT_TIMER_timer_sleep(volatile struct timer* timer_item, float period) {
    uint16_t duration = 0;
    timer_command t_command = TIMER_SLEEP;
    if (timer_item->command == TIMER_STOP) {
        timer_item->command = t_command;
        timer_item->runtime = duration/period;
        QT_TIMER_setup(timer_item, duration, period);
        return 1;
    } else {
        return 0;
    }
}

volatile struct timer* QT_TIMER_startPeriodicTask(timer_command t_command, uint16_t duration, float period) {
    if (t_command == timer0.command || t_command == timer1.command ||t_command == timer2.command || t_command == timer3.command) {
        return NULL;

    } else if (timer0.command == TIMER_STOP) {
        timer0.command = t_command;
        timer0.runtime = duration/period;
        QT_TIMER_setup(&timer0, duration, period);
        return &timer0;

    } else if (timer1.command == TIMER_STOP) {
        timer1.command = t_command;
        timer1.runtime = duration/period;
        QT_TIMER_setup(&timer1, duration, period);
        return &timer1;

    } else if (timer2.command == TIMER_STOP) {
        timer2.command = t_command;
        timer2.runtime = duration/period;
        QT_TIMER_setup(&timer2, duration, period);
        return &timer2;

    } else if (timer3.command == TIMER_STOP) {
        timer3.command = t_command;
        timer3.runtime = duration/period;
        QT_TIMER_setup(&timer3, duration, period);
        return &timer3;
    }
    return NULL;
}

void QT_TIMER_stopPeriodicTask(volatile struct timer *timer_item) {
    int timer_no = timer_item->id;
    switch (timer_no) {
    case 0:
        TB0CCR0 = 0;
        TB0CCTL0 = 0;
        TB0CTL = MC_0;
        break;
    case 1:
        TB1CCR0 = 0;
        TB1CCTL0 = 0;
        TB1CTL = MC_0;
        break;
    case 2:
        TB2CCR0 = 0;
        TB2CCTL0 = 0;
        TB2CTL = MC_0;
        break;
    case 3:
        TB3CCR0 = 0;
        TB3CCTL0 = 0;
        TB3CTL = MC_0;
        break;
    }
//    resetTaskFlag(timer_item->command);
    timer_item->command = TIMER_STOP;
    timer_item->runtime = 0;
    timer_item->duty_period = 0;
    P1OUT =0;
}

void QT_TIMER_resetTaskFlag(timer_command t_command) {
    switch (t_command) {
    case DEPLOY_PROBE_SP:
        break;
    case DEPLOY_PROBE_FP:
        break;
    }
}

volatile struct timer* QT_TIMER_startPWM(timer_command t_command, uint16_t duration, float period, uint16_t duty) {

    volatile struct timer *pwm_timer = QT_TIMER_startPeriodicTask(t_command, 2*duration, period);
    if (pwm_timer == NULL) {return NULL;}
    float duty_p = duty* 0.01 * pwm_timer->period;
    pwm_timer->duty_period = (uint16_t) duty_p;
    return pwm_timer;
}

static void QT_TIMER_handlePeriodicTask(volatile struct timer *timer_item) {
    if (timer_item->runtime > 0) {
        timer_item->runtime --;
    } else {
        QT_TIMER_stopPeriodicTask(timer_item);
    }
    timer_command t_command = timer_item->command;
    switch (t_command) {
    case DEPLOY_PROBE_SP:
        QT_BW_toggleSpBurnwire();
        break;
    case DEPLOY_PROBE_FP:
        QT_BW_toggleFpBurnwire();
        break;
    case TIMER_SLEEP:
        break;
    case SAMPLE_PROBE:
        sweepFlag = true;
        break;
    case SET_DAC:
        dacFlag = true;
        break;
    }

}

#pragma vector = TIMER0_B0_VECTOR
__interrupt void timer_b0(void) {
    QT_TIMER_handlePeriodicTask(&timer0);
    if (timer0.duty_period != 0) {
        if (TB0CCR0 == timer0.duty_period) {
            TB0CCR0 = timer0.period - timer0.duty_period;
        } else {
            TB0CCR0 = timer0.duty_period;
        }
    }
}

#pragma vector = TIMER1_B0_VECTOR
__interrupt void timer_b1(void) {
    QT_TIMER_handlePeriodicTask(&timer1);
    if (timer1.duty_period != 0) {
        if (TB1CCR0 == timer1.duty_period) {
            TB1CCR0 = timer1.period - timer1.duty_period;
        } else {
            TB1CCR0 = timer1.duty_period;
        }
    }
}

#pragma vector = TIMER2_B0_VECTOR
__interrupt void timer_b2(void) {
    QT_TIMER_handlePeriodicTask(&timer2);
    if (timer2.duty_period != 0) {
        if (TB2CCR0 == timer2.duty_period) {
            TB2CCR0 = timer2.period - timer2.duty_period;
        } else {
            TB2CCR0 = timer2.duty_period;
        }
    }
}

#pragma vector = TIMER3_B0_VECTOR
__interrupt void timer_b3(void) {
    QT_TIMER_handlePeriodicTask(&timer3);
    if (timer3.duty_period != 0) {
        if (TB3CCR0 == timer3.duty_period) {
            TB3CCR0 = timer3.period - timer3.duty_period;
        } else {
            TB2CCR0 = timer3.duty_period;
        }
    }
}

