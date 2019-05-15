//#ifndef ASPILIB_H
//#define ASPILIB_H
//
///*
// * Includes
// */
//#include "Common/QT_COM_common.h"
//
//#define CPOL(x) (x ? UCCKPL : 0x00)
//#define CPHA(x) (x ? 0x00 : UCCKPH)
//
//typedef struct {
//    bool isSlave;
//    uint16_t spiBaseAddress;
//    receive_handler_func receiveHandler;
//    byte expectedLength;
//
//    // CS storage
//    uint8_t csPort, csPin;
//    bool cpol, cpha;
//} device_t;
