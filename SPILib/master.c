#include "SpiLib/QT_SPI_SpiLib.h"
#include "BurnWire/QT_BW_BurnWire.h"
#include "InternalADC/QT_adc.h"

#include "driverlib.h"

#define DATA_LENGTH 128

byte dataPtr[DATA_LENGTH];

void handler(bool success) {
    QT_SPI_transmit(dataPtr, DATA_LENGTH, &ADC, handler);
}

void main(void)
{
    int i;

    for(i = 0; i < DATA_LENGTH; i++) {
        dataPtr[i] = (byte) ('A' + (i % 26));
    }

    // Stop watchdog timer
    WDT_A_hold(WDT_A_BASE);

    QT_SPI_initialise();
    QT_BW_initialise();
    QT_ADC_initialise();

    // Disable the GPIO power-on default high-impedance mode
    // to activate previously configured port settings
    PMM_unlockLPM5();

    // Enable interrupts
    __bis_SR_register(GIE);

    i = 0;

    QT_SPI_transmit(dataPtr, DATA_LENGTH, &ADC, handler);
    QT_BW_deploy();

    while(1) {  }
}
