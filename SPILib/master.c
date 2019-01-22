#include "spilib.h"
#include "driverlib.h"

#define DATA_LENGTH 128

byte dataPtr[DATA_LENGTH];

void handler(const byte *data) {
    int i;
    byte d [100];

    GPIO_toggleOutputOnPin(GPIO_PORT_P1, GPIO_PIN1);

    for(i = 0; i < 100; i++) {
        d[i] = data[i];
    }

    i = 5;
}

void main(void)
{
    int i;

    for(i = 0; i < DATA_LENGTH; i++) {
        //dataPtr[i] = (byte) ('A' + (i % 26));
        dataPtr[i] = (byte) 0b1001001;
    }

    // Stop watchdog timer
    WDT_A_hold(WDT_A_BASE);

    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);

    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN1);
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN1);

    QT_SPI_initialise();

    // Disable the GPIO power-on default high-impedance mode
    // to activate previously configured port settings
    PMM_unlockLPM5();

    // Enable interrupts
    __bis_SR_register(GIE);

    i = 0;

    QT_SPI_setReceiveHandler(handler, 100, &OBC);

    while (1) {
        GPIO_toggleOutputOnPin(GPIO_PORT_P1, GPIO_PIN0);
        __delay_cycles(500000);
    }
}
