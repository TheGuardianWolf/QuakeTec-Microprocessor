///*
// * QT_digipot.c
// *
// *  Created on: 16/01/2019
// *      Author: james
// */
//
//#include <stdlib.h>
//
//#include "driverlib.h"
//#include "SpiLib/QT_SPI_SpiLib.h"
//
//static uint16_t registerValue;
//
//#define CONTROL_SHIFT 10
//
//#define DIGIPOT_RDY_PORT GPIO_PORT_P2
//#define DIGIPOT_RDY_PIN GPIO_PIN7
//
//#define DIGIPOT_MAX_DATA 1024.0
//#define DIGIPOT_SCALE (9900.0 / 20000.0 * DIGIPOT_MAX_DATA)
//#define DIGIPOT_WRITE_COMMAND 1
//
//static void setReady(bool high) {
//    if (high) {
//        GPIO_setOutputHighOnPin(DIGIPOT_RDY_PORT, DIGIPOT_RDY_PIN);
//    } else {
//        GPIO_setOutputLowOnPin(DIGIPOT_RDY_PORT, DIGIPOT_RDY_PIN);
//    }
//}
//
///**
// * This sends the whole register to the digipot, control and data bits.
// *
// * If the SPI line is busy this method waits until it is free.
// **/
////static void syncRegister() {
////    // TODO validate timings with o-scope
////    // We need to make this so the method does not block for extended periods of time.
////
////    setReady(true);
////    while(!QT_SPI_transmit((byte *) &registerValue, 2, &DIGIPOT, NULL));
////    while(!QT_SPI_isDataSent());
////    setReady(false);
////}
//
//void QT_DIGIPOT_setGain(float gain) {
//    uint16_t data = (uint16_t) (DIGIPOT_SCALE / (gain - 1.0));
//
//    if(data >= DIGIPOT_MAX_DATA) {
//        data = DIGIPOT_MAX_DATA - 1;
//    } else if(data < 0) {
//        data = 0;
//    }
//
//    uint8_t control = DIGIPOT_WRITE_COMMAND;
//
//    registerValue = data | (control << CONTROL_SHIFT);
//    syncRegister();
//}
