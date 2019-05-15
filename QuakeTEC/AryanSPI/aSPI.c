//#include "SpiLib/QT_SPI_SpiLib.h"
//
//#define SPI_CLK_FREQUENCY 50000
//
//device_t OBC = { false, EUSCI_A1_BASE, NULL, 0, GPIO_PORT_P4, GPIO_PIN0, true, true };
//
//device_t ADC = { true, EUSCI_B1_BASE, NULL, 0, GPIO_PORT_P2, GPIO_PIN5, true, true };
//device_t DAC = { true, EUSCI_B1_BASE, NULL, 0, GPIO_PORT_P2, GPIO_PIN4, true, true };
//device_t DIGIPOT = { true, EUSCI_B1_BASE, NULL, 0, GPIO_PORT_P2, GPIO_PIN6, false, true };
//
//
//
//
//#pragma vector=USCI_B1_VECTOR
//__interrupt void USCI_B1_ISR(void)
//{
//    switch(UCB1IV)
//    {
//    // Receive data case
//    case USCI_SPI_UCRXIFG:
//        // Check if we're pointing to a slave
//        if (currentSlavePtr == NULL) {
//            break;
//        }
//
//        receiveByte(currentSlavePtr, &EUSCI_B_SPI_receiveData, &slaveReceiveBufferPtr, slaveReceiveBuffer);
//
//        if(slaveCSShutoff) {
//            setChipSelect(NULL);
//            slaveCSShutoff = false;
//        }
//
//        break;
//
//        // Transmit data case
//    case USCI_SPI_UCTXIFG:
//        isSlaveTransmitInterruptPending = false;
//
//        if (currentSlavePtr == NULL) {
//            break;
//        }
//
//        sendByteToSlave();
//
//        break;
//
//    default:
//        break;
//    }
//}
//
///**
// * Interrupt for processing transmitted and received data from OBC
// */
//#pragma vector=USCI_A1_VECTOR
//__interrupt
//void USCI_A1_ISR (void)
//{
//    switch(UCA1IV)
//        {
//            // Receive data case
//            case USCI_SPI_UCRXIFG:
//                // Receive data from master
//                if(isListeningToMaster) {
//                    receiveByte(&OBC, &EUSCI_A_SPI_receiveData, &masterReceiveBufferPtr, masterReceiveBuffer);
//                }
//
//                setMasterListenState(requestedMasterListenState);
//
//                break;
//
//            // Transmit data case
//            case USCI_SPI_UCTXIFG:
//                setMasterListenState(requestedMasterListenState);
//
//                // Transmit data to master
//                sendByteToMaster();
//
//                break;
//
//            default:
//                break;
//        }
//}
