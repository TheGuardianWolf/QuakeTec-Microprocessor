#include "spilib.h"
#include "driverlib.h"

unsigned char TXData = 0;
unsigned char RXData = 0;

void handler(const byte *data) {
    GPIO_toggleOutputOnPin(GPIO_PORT_P1, GPIO_PIN0);
}

void main(void)
{
    // Stop watchdog timer
    WDT_A_hold(WDT_A_BASE);

    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);

    initialise();

    // Disable the GPIO power-on default high-impedance mode
    // to activate previously configured port settings
    PMM_unlockLPM5();

    // Enable interrupts
    __bis_SR_register(GIE);

    setReceiveHandler(handler, 32, &DIGIPOT);
    listenToSlave(&DIGIPOT);
}
