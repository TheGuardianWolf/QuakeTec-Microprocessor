/*
 * Includes
 */
#include <string.h>
#include <SpiLib/QT_SPI_SpiLib.h>

/*
 * Defines
 */

#define TX_HEADER_SIZE 2
#define BUFFER_SIZE 255

static QT_OBCSPI_ISRRxHandler_t rxHandler = NULL;
static QT_OBCSPI_ISRTxHandler_t txHandler = NULL;
static uint8_t txData[TX_HEADER_SIZE + BUFFER_SIZE] = { 0 };
static uint16_t txDataLength = 0;
static uint16_t txDataSent = 0;
static uint8_t rxData[BUFFER_SIZE] = { 0 };
static uint16_t rxDataLength = 0;

/*
 * Private variables
 */

/*
 * Public functions
 */

/**
 * Sets up the SPI library
 */
void QT_OBCSPI_init() {
    uint8_t port = GPIO_PORT_P4;
    uint8_t mosi = GPIO_PIN3;
    uint8_t miso = GPIO_PIN2;
    uint8_t clk = GPIO_PIN1;

    // Set OBC chip select as input
    GPIO_setAsInputPin(GPIO_PORT_P4, GPIO_PIN0);

    // Set MOSI and CLK lines as input
    GPIO_setAsPeripheralModuleFunctionInputPin(
            port,
            mosi + clk,
            GPIO_PRIMARY_MODULE_FUNCTION
    );

    // Set MISO line as output
    GPIO_setAsPeripheralModuleFunctionOutputPin(
            port,
            miso,
            GPIO_PRIMARY_MODULE_FUNCTION
    );

    // Initialize slave to MSB first, inactive high clock polarity and 4 wire SPI
    EUSCI_A_SPI_initSlaveParam param = {0};
    param.msbFirst = EUSCI_A_SPI_MSB_FIRST;
    param.clockPhase = EUSCI_A_SPI_PHASE_DATA_CHANGED_ONFIRST_CAPTURED_ON_NEXT;
    param.clockPolarity = EUSCI_A_SPI_CLOCKPOLARITY_INACTIVITY_HIGH;
    param.spiMode = EUSCI_A_SPI_4PIN_UCxSTE_ACTIVE_LOW;
    EUSCI_A_SPI_initSlave(EUSCI_A1_BASE, &param);

    // Select 4 pin functionality
    EUSCI_A_SPI_select4PinFunctionality(EUSCI_A1_BASE, EUSCI_A_SPI_ENABLE_SIGNAL_FOR_4WIRE_SLAVE);

    // Enable SPI Module
    EUSCI_A_SPI_enable(EUSCI_A1_BASE);

    // Ensure first transmit is set
    EUSCI_A_SPI_transmitData(EUSCI_A1_BASE, 0);

    // Clear receive interrupt flag for communication with OBC
    EUSCI_A_SPI_clearInterrupt(EUSCI_A1_BASE,
                               EUSCI_A_SPI_RECEIVE_INTERRUPT);

    // Enable receive interrupt
    EUSCI_A_SPI_enableInterrupt(EUSCI_A1_BASE,
                                EUSCI_A_SPI_RECEIVE_INTERRUPT);
}

void QT_OBCSPI_disableInterrupts()
{
    EUSCI_A_SPI_disableInterrupt(EUSCI_A0_BASE, EUSCI_A_SPI_RECEIVE_INTERRUPT);
}

void QT_OBCSPI_enableInterrupts()
{
    EUSCI_A_SPI_enableInterrupt(EUSCI_A0_BASE, EUSCI_A_SPI_RECEIVE_INTERRUPT);
}

void QT_OBCSPI_clearRx()
{
    rxDataLength = 0;
}

void QT_OBCSPI_clearTx()
{
    txDataLength = 0;
    txDataSent = 0;
}

void QT_OBCSPI_writeTx(uint8_t* data, uint16_t dataLength)
{
    if (dataLength < BUFFER_SIZE)
    {
        txData[0] = PL_START_BYTE;
        txData[1] = (uint8_t) dataLength;
        memcpy(&txData[2], data, dataLength);
        txDataLength = dataLength + TX_HEADER_SIZE;
        txDataSent = 0;
    }
}

void QT_OBCSPI_registerISRHandlers(QT_OBCSPI_ISRRxHandler_t handler1, QT_OBCSPI_ISRTxHandler_t handler2)
{
    rxHandler = handler1;
    txHandler = handler2;
}


/**
 * Interrupt for processing transmitted and received data from OBC
 */
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCI_A1_VECTOR
__interrupt
#elif defined(__GNUC__)
__attribute__((interrupt(USCI_A1_VECTOR)))
#endif
void USCI_A1_ISR (void)
{
    switch(__even_in_range(UCA1IV, USCI_SPI_UCTXIFG))
    {
        // Receive data case
        case USCI_SPI_UCRXIFG:
        {
            // Wait TX Ready
            while (!EUSCI_A_SPI_getInterruptStatus(EUSCI_A1_BASE,
                                                   EUSCI_A_SPI_TRANSMIT_INTERRUPT
                                                    ));
            // Read into buffer
            uint8_t receivedData = EUSCI_A_SPI_receiveData(EUSCI_A1_BASE);
            if (rxDataLength < BUFFER_SIZE)
            {
                rxData[rxDataLength] = receivedData;
                rxDataLength += 1;
            }

            // Handle Rx
            if (rxHandler != NULL)
            {
                (*rxHandler)(rxData, rxDataLength);
            }

            // Transmit from buffer
            if (txDataSent < txDataLength)
            {
                EUSCI_A_SPI_transmitData(EUSCI_A1_BASE, txData[txDataSent]);
                txDataSent += 1;
            }
            else
            {
//                EUSCI_A_SPI_transmitData(EUSCI_A1_BASE, receivedData);
                EUSCI_A_SPI_transmitData(EUSCI_A1_BASE, receivedData);
            }

            // Handle Tx
            if (txHandler != NULL)
            {
                (*txHandler)(txData, txDataLength, txDataSent);
            }
        }
        break;
        default:
        break;
    }
}
