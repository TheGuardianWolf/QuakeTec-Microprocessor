/*
 * Includes
 */

#include "driverlib.h"
#include <stddef.h>
#include "spilib.h"

/*
 * Defines
 */
#define BUFFER_LENGTH 256
#define SPI_CLK_FREQUENCY 500
#define DEFAULT_SEND 0

/*
 * Private variables
 */

/**
 * SPI related information about devices to communicate with.
 */
device_t OBC = { false, 0, NULL, 0 };
device_t ADC = { true, 0, NULL, 0 };
device_t DAC = { true, 0, NULL, 0 };
device_t DIGIPOT = { true, EUSCI_A0_BASE, NULL, 0 };

/**
 * Buffers to hold data before data is ready to send to receive handler
 */
static byte masterReceiveBuffer [BUFFER_LENGTH];
static byte slaveReceiveBuffer [BUFFER_LENGTH];

/**
 * Pointers to next available byte in the buffer. If buffer is full, pointer will point to byte 256.
 */
static byte *masterReceiveBufferPtr;
static byte *slaveReceiveBufferPtr;

/**
 * Pointer to slave device MPS430 communicating with.
 */
static device_t *currentSlavePtr;

/**
 * Whether we are currently listening to the slave.
 */
static bool isListening;

/**
 * Pointer to the next byte to be sent
 */
static const byte *masterTransmitPtr;
static const byte *slaveTransmitPtr;

/**
 * Pointers one past the last byte to be sent
 */
static const byte *masterTransmitStopPtr, *slaveTransmitStopPtr;

/*
 * Private functions
 */

/**
 * Sets up the MSP430 as a master device for communication with ADC, DAC and DigiPot.
 */
static void initialiseMaster() {

    // Select Port 1 - Set Pin 0, Pin 1 and Pin 2 to input Secondary Module Function
    GPIO_setAsPeripheralModuleFunctionInputPin(
            GPIO_PORT_P1,
            GPIO_PIN7 + GPIO_PIN6 + GPIO_PIN5 + GPIO_PIN4,
            GPIO_PRIMARY_MODULE_FUNCTION
    );

    // Initialize Master
    EUSCI_A_SPI_initMasterParam param = {0};
    param.selectClockSource = EUSCI_A_SPI_CLOCKSOURCE_SMCLK;
    param.clockSourceFrequency = CS_getSMCLK();
    param.desiredSpiClock = SPI_CLK_FREQUENCY;
    param.msbFirst = EUSCI_A_SPI_MSB_FIRST;
    param.clockPhase = EUSCI_A_SPI_PHASE_DATA_CHANGED_ONFIRST_CAPTURED_ON_NEXT;
    param.clockPolarity = EUSCI_A_SPI_CLOCKPOLARITY_INACTIVITY_HIGH;
    param.spiMode = EUSCI_A_SPI_3PIN;
    EUSCI_A_SPI_initMaster(EUSCI_A0_BASE, &param);

    // Enable SPI module
    EUSCI_A_SPI_enable(EUSCI_A0_BASE);

    // Clear receive interrupt flag
    EUSCI_A_SPI_clearInterrupt(EUSCI_A0_BASE,
                               EUSCI_A_SPI_RECEIVE_INTERRUPT | EUSCI_A_SPI_TRANSMIT_INTERRUPT
    );

    // Enable USCI_A0 RX interrupt
    EUSCI_A_SPI_enableInterrupt(EUSCI_A0_BASE,
                                EUSCI_A_SPI_RECEIVE_INTERRUPT | EUSCI_A_SPI_TRANSMIT_INTERRUPT);

    // Wait for slave to initialize
    __delay_cycles(100);
}

/**
 * Sets up MSP430 as slave device for communication with OBC.
 */
static void initialiseSlave() {

}

/**
 * Sets the MSP430 to listen to the specified device.
 */
static void setChipSelect(device_t* devicePtr) {

}

/*
 * Public functions
 */

/**
 * Sets up the SPI library
 */
void initialise() {

    // Set receive buffer to point to start of buffer
    masterReceiveBufferPtr = masterReceiveBuffer;
    slaveReceiveBufferPtr = slaveReceiveBuffer;

    // Set slave device we are listening to as null
    currentSlavePtr = NULL;

    // Set up the send buffers
    slaveTransmitPtr = NULL;
    masterTransmitPtr = NULL;

    // Setup listening
    isListening = false;

    // Set base addresses

    initialiseMaster();

    initialiseSlave();
}

/**
 * Returns immediately. Returns true if the system was ready to send data. If it returns false the data was not sent.
 */
bool transmit(const byte *dataPtr, byte length, device_t *devicePtr) {
    // Check if we are currently sending, if we are return false
    if (devicePtr->isSlave) {
        if (slaveTransmitPtr != NULL) {
            return false;
        }
    } else {
        if (masterTransmitPtr != NULL) {
            return false;
        }
    }

    // Check if we are connected to the correct slave, else change and clear
    if (devicePtr->isSlave && currentSlavePtr != devicePtr) {
        slaveReceiveBufferPtr = slaveReceiveBuffer;
        setChipSelect(currentSlavePtr);
        isListening = false;
    }

    // Set the transmit buffer
    if (devicePtr->isSlave) {
        slaveTransmitPtr = dataPtr;
        slaveTransmitStopPtr = dataPtr + length;
    } else {
        masterTransmitPtr = dataPtr;
        masterTransmitStopPtr = dataPtr + length;
    }

    return true;
}

/**
 * Registers a function that handles incoming data. This will be called when length bytes have been recevied.
 */
void setReceiveHandler(handler_func handler, byte length, device_t *devicePtr) {

    // Disable receive interrupts (interrupts will be queued for processing afterwards)
    EUSCI_A_SPI_disableInterrupt(devicePtr->spiBaseAddress, EUSCI_A_SPI_RECEIVE_INTERRUPT);

    // Clear the receive buffers
    masterReceiveBufferPtr = masterReceiveBuffer;
    slaveReceiveBufferPtr = slaveReceiveBuffer;

    // Set receive handler and expected length
    devicePtr->receiveHandler = handler;
    devicePtr->expectedLength = length;

    // Re-enable receive interrupts
    EUSCI_A_SPI_enableInterrupt(devicePtr->spiBaseAddress, EUSCI_A_SPI_RECEIVE_INTERRUPT);

    // Run queued interrupts

}

/**
 * Starts clocking data in from the slave and sets the CS line. This will cause the handler to be called.
 * If there is data in the receive buffer, clear it.
 */
void listenToSlave(device_t *devicePtr) {

    // Clear data in receive buffer
    slaveReceiveBufferPtr = slaveReceiveBuffer;

    // Store the new slave to listen to
    currentSlavePtr = devicePtr;

    // Set the chip select
    setChipSelect(devicePtr);

    isListening = true;

    // TODO
    // TESTING: transmit default value to trigger receive interrupt
    EUSCI_A_SPI_transmitData(currentSlavePtr->spiBaseAddress, DEFAULT_SEND);
}

/**
 * Stops listening to the current slave.
 */
void stopListeningToSlave() {
    isListening = false;
}

/**
 * Returns true if all data has been sent.
 */
bool isDataSent() {
    return masterTransmitPtr == NULL && slaveTransmitPtr == NULL;
}

/*
 * Interrupts
 */

/**
 * Interrupt for processing received and transmitted data from the A0 channel (from slave devices)
 */
#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
{
    // Check if slave has been set
    if (currentSlavePtr == NULL) {
        return;
    }

    switch(UCA0IV)
    {
    // Receive data case
    case USCI_SPI_UCRXIFG: {

        // Check if receive handler has been set
        if (currentSlavePtr->receiveHandler == NULL) {
            return;
        }

        // Put received byte of data in slave buffer
        *slaveReceiveBufferPtr = EUSCI_A_SPI_receiveData(EUSCI_A0_BASE);

        // Move buffer pointer to next byte
        slaveReceiveBufferPtr++;

        // If the buffer is full
        if (slaveReceiveBufferPtr - slaveReceiveBuffer >= currentSlavePtr->expectedLength) {

            // Run the handler
            (*(currentSlavePtr->receiveHandler))(slaveReceiveBuffer);

            // Clear the buffer
            slaveReceiveBufferPtr = slaveReceiveBuffer;
        }

        break;
    }
    // Transmit data case
    case USCI_SPI_UCTXIFG: {
        if (slaveTransmitPtr == NULL) {
            // If we are listening, but not sending, send 0.
            if (isListening) {
                EUSCI_A_SPI_transmitData(currentSlavePtr->spiBaseAddress, DEFAULT_SEND);
                return;
            }
        }

        // If we have data to send, send it.
        byte newData = *(slaveTransmitPtr++);

        if (slaveTransmitPtr == slaveTransmitStopPtr) {
            slaveTransmitPtr = NULL;
        }

        EUSCI_A_SPI_transmitData(currentSlavePtr->spiBaseAddress, newData);

        break;
    }

    default:
        break;
    }
}
