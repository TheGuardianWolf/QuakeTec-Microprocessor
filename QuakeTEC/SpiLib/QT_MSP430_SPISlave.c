//#include "QT_MSP430_SPISlave.h"
//#include "../driverlib/MSP430FR2xx_4xx/driverlib.h"
//#include "../PL_Protocol.h"
//
//#define PORT   GPIO_PORT_P4
//#define MOSI   GPIO_PIN3
//#define MISO   GPIO_PIN2
//#define CLK    GPIO_PIN1
//#define CS     GPIO_PIN0
//
//#define TX_BUFFER_LENGTH 255
//#define RX_BUFFER_LENGTH 255
//
///*
// * Ring buffer for data to be transmitted to master.
// * u16_TxReadPos contains the current position in the buffer which we are reading bytes to be transmitted from.
// * u16_TxWritePos contains the current position in the buffer to which we will write the next byte to queue for transmission.
// * These values should be reset to 0 if they ever equal TX_BUFFER_LENGTH.
// * If u16_TxWritePos reaches u16_TxReadPos, then the buffer is full.
// * If u16_TxReadPos reaches u16_TxWritePos, then the buffer is empty.
// */
//volatile uint8_t ru8_TxBuffer[TX_BUFFER_LENGTH];
//volatile uint16_t u16_TxReadPos = 0;
//volatile uint16_t u16_TxWritePos = 0;
//volatile uint16_t u16_TxBytes = 0;
//
///*
// * Ring buffer for data received from master.
// * See comment for previous ring buffer for details.
// */
//volatile uint8_t ru8_RxBuffer[RX_BUFFER_LENGTH];
//volatile uint16_t u16_RxReadPos = 0;
//volatile uint16_t u16_RxWritePos = 0;
//volatile uint16_t u16_RxBytes = 0;
//
///*--------------------------------------------------------------------------------
// * Private functions.
// *--------------------------------------------------------------------------------*/
//
///*
// * Saves the last received byte in ru8_RxBuffer.
// */
//inline void ReceiveByte()
//{
//
//    /* Do not add byte if buffer is full. */
//    if (u16_RxBytes == RX_BUFFER_LENGTH) return;
//
//    /* Load the new byte into the Tx buffer. */
//    ru8_RxBuffer[u16_RxWritePos] = (uint8_t) HWREG16(UCA1RXBUF);
//
//    /* Move the write position to the next byte. */
//    u16_RxWritePos++;
//    if (u16_RxWritePos > RX_BUFFER_LENGTH)
//    {
//       u16_RxWritePos = 0;
//    }
//
//    /* Increment the count of bytes in the buffer. */
//    u16_RxBytes++;
//}
//
///*
// * Sends the next byte in the ru8_TxBuffer.
// */
//inline void SendByte()
//{
//    /* Do nothing if the next byte hasn't been written yet. */
//    if (u16_TxReadPos == u16_TxWritePos) return;
//
//    /* Load the current byte from the transmit buffer to be transmitted. */
//    HWREG16(UCA1TXBUF) = (uint16_t) ru8_TxBuffer[u16_TxReadPos];
//
//    /* Move the read position to the next byte. */
//    u16_TxReadPos++;
//    if (u16_TxReadPos > TX_BUFFER_LENGTH)
//    {
//       u16_TxReadPos = 0;
//    }
//}
//
///*--------------------------------------------------------------------------------
// * Interrupts.
// *--------------------------------------------------------------------------------*/
//
///*
// * SPI Tx/Rx complete ISR.
// *
// * It would have been nice to use a pin change interrupt, but the CS pin we would watch
// * is on the same port as all of the rapidly changing data pins from both SPI buses. In
// * future, it would be wise to keep all pins of significant interest (eg CS) on a single
// * port, so that a simpler, blocking SPI routine could run when CS is pulled low.
// */
//#pragma vector=USCI_A1_VECTOR
//__interrupt
//void USCI_A1_ISR()
//{
//    switch (UCA1IV)
//    {
//    /* Rx complete. */
//    case USCI_SPI_UCRXIFG:
//        ReceiveByte(); // This will never allow overrn of our buffer, but will drop bytes when the buffer is full.
//        break;
//
//    /* Tx complete. */
//    case USCI_SPI_UCTXIFG:
//        SendByte();
//        break;
//    }
//}
//
///*--------------------------------------------------------------------------------
// * Public functions.
// *--------------------------------------------------------------------------------*/
//
//void SPISlave_Init()
//{
//    /* Configure CS as input with pull-up resistor. */
//    GPIO_setAsInputPin(PORT, CS);
//    P4DIR &= ~(CS);
//    P4REN |= CS;
//    P4OUT |= CS;
//
//    /* Configure MOSI and CLK. */
//    GPIO_setAsPeripheralModuleFunctionInputPin(
//        PORT,
//        MOSI | CLK,
//        GPIO_PRIMARY_MODULE_FUNCTION
//    );
//
//    /* Configure MISO. */
//    GPIO_setAsPeripheralModuleFunctionOutputPin(
//        PORT,
//        MISO,
//        GPIO_PRIMARY_MODULE_FUNCTION
//    );
//
//    /* Disable device during configuration. */
//    HWREG16(UCA1CTLW0) |= UCSWRST;
//
//    /* Clear control register (except UCSWRST bit). */
//    HWREG16(UCA1CTLW0) &= ~(0xfffe);
//
//    /* Configure control register as desired. */
//    HWREG16(UCA1CTLW0) |= (
//        0x0000
//        //| UCCKPH  // Clock phase. We want this unset for capture on next.
//        | UCCKPL  // Clock polarity. We want this set for inactive high.
//        | UCMSB   // MSB first. We want this set.
//        //| UC7BIT  // 7bit character length. We want this unset for 8 bit.
//        //| UCMST     // Master mode. We want this unset for slave mode.
//        | UCMODE1 // eUSCI mode bit 1. We want 10 for 4 pin SPI with UCxSTE active low.
//        //| UCMODE0 // eUSCI mode bit 0.
//        //| UCSYNC  // Synchronous mode.
//        //| UCSSEL1   // Clock source select bit 1. Irrelevant for slave.
//        //| UCSSEL0   // Clock source select bit 0.
//        //| UCSTEM  // STE mode select for master mode. Irrelevant in slave mode or 3 wire mode.
//    );
//
//    /* Re-enable device. */
//    HWREG16(UCA1CTLW0) &= ~(UCSWRST);
//
//    /* Clear interrupt flags. */
//    HWREG16(UCA1IFG) = ~(UCTXIE | UCRXIE);
//
//    /* Enable interrupts. */
//    HWREG16(UCA1IE) |= UCTXIE | UCRXIE;
//}
//
//void SPISlave_DisableInterrupts()
//{
//    HWREG16(UCA1IE) &= ~(UCTXIE | UCRXIE);
//}
//
//void SPISlave_EnableInterrupts()
//{
//    HWREG16(UCA1IE) |= UCTXIE | UCRXIE;
//}
//
//uint8_t SPISlave_SendByte(uint8_t u8_ByteToSend)
//{
//    /* Do not queue byte if buffer is full. */
//    if (u16_TxBytes == TX_BUFFER_LENGTH) return 1;
//
//    /* Load the new byte into the Tx buffer. */
//    ru8_TxBuffer[u16_TxWritePos] = u8_ByteToSend;
//
//    /* Move the write position to the next byte. */
//    u16_TxWritePos++;
//    if (u16_TxWritePos > TX_BUFFER_LENGTH)
//    {
//        u16_TxWritePos = 0;
//    }
//
//    /* Increment the count of bytes in the buffer. */
//    u16_TxBytes++;
//
//    return 0;
//}
//
//uint8_t SPISlave_Example_GetNextRxByte(uint8_t *pu8_Byte)
//{
//    /* This is an example of how you might retrieve the latest byte received. If
//     * you were to call this several times, it would retrieve bytes in the order
//     * they were received. */
//
//    /* Do nothing if the next byte doesn't exist yet. */
//    if (u16_RxReadPos == u16_RxWritePos) return 1;
//
//    /* Extract the value of the byte at the current read position. */
//    *pu8_Byte = ru8_RxBuffer[u16_RxReadPos];
//
//    /* Move the read position to the next byte. */
//    u16_RxReadPos++;
//    if (u16_RxReadPos > RX_BUFFER_LENGTH)
//    {
//      u16_RxReadPos = 0;
//    }
//
//    return 0;
//}
