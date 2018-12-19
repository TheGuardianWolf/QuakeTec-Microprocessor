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
#define SPI_CLK_FREQUENCY 2000
#define DEFAULT_SEND 0

/*
 * Private variables
 */

/**
 * SPI related information about devices to communicate with.
 */
device_t OBC = { false, EUSCI_A0_BASE, NULL, 0, GPIO_PORT_P1, GPIO_PIN4 };
device_t ADC = { true, EUSCI_A0_BASE, NULL, 0, GPIO_PORT_P1, GPIO_PIN4 };
device_t DAC = { true, EUSCI_A0_BASE, NULL, 0, GPIO_PORT_P1, GPIO_PIN4 };
device_t DIGIPOT = { true, EUSCI_A0_BASE, NULL, 0, GPIO_PORT_P1, GPIO_PIN4 };

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
static bool isCurrentlySending = false;

/**
 * Pointer to the next byte to be sent, this will be NULL if there is no data to send
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
    const uint8_t port = GPIO_PORT_P1;
    const uint8_t mosi = GPIO_PIN7;
    const uint8_t miso = GPIO_PIN6;
    const uint8_t clk = GPIO_PIN5;

    // Set all of the device chip selects at output
    //GPIO_setAsOutputPin(DIGIPOT.csPort, DIGIPOT.csPort);

    // Set chip select high
    //GPIO_setOutputHighOnPin(DIGIPOT.csPort, DIGIPOT.csPort);

    // Select Port 1 - Set Pin 0, Pin 1 and Pin 2 to input Secondary Module Function
    GPIO_setAsPeripheralModuleFunctionInputPin(
            port,
            miso,
            GPIO_PRIMARY_MODULE_FUNCTION
    );

    GPIO_setAsPeripheralModuleFunctionOutputPin(
            port,
            mosi | clk,
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
 * Sets the MSP430 to listen to the specified device. If null is passed, then it will clear all chip selects
 *
 * Don't change the currentSlavePtr manually
 */
static void setChipSelect(device_t* devicePtr) {
    if (currentSlavePtr != NULL) {
        // Clear cs
        GPIO_setOutputHighOnPin(currentSlavePtr->csPort, currentSlavePtr->csPin);
    }

    currentSlavePtr = devicePtr;

    if (currentSlavePtr != NULL) {
        // Set new cs
        GPIO_setOutputLowOnPin(currentSlavePtr->csPort, currentSlavePtr->csPin);
    }
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
 * This method sends a single byte to the slave. This method also sets the isCurrentlySending flag.
 */
static void sendByteToSlave() {
    if (slaveTransmitPtr == NULL) {
        isCurrentlySending = false;
        return;
    }

    EUSCI_A_SPI_transmitData(currentSlavePtr->spiBaseAddress, *slaveTransmitPtr);
    isCurrentlySending = true;
    slaveTransmitPtr++;

    if (slaveTransmitPtr == slaveTransmitStopPtr) {
        slaveTransmitPtr = NULL;
    }
}

/**
 * Returns immediately. Returns true if the system was ready to send data. If it returns false the data was not sent.
 */
bool transmit(const byte *dataPtr, uint16_t length, device_t *devicePtr) {

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
        // Clear the receive buffer
        slaveReceiveBufferPtr = slaveReceiveBuffer;

        // Select the device
        setChipSelect(devicePtr);

        // Clear the listening flag
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

    if (!isCurrentlySending) {

        // Start interrupts
        sendByteToSlave();
    }

    return true;
}

/**
 * This method sends a single 0 to the slave. This method also sets the isCurrentlySending flag.
 */
static void sendNullByteToSlave() {
    EUSCI_A_SPI_transmitData(currentSlavePtr->spiBaseAddress, DEFAULT_SEND);
    isCurrentlySending = true;
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

    // Set the chip select
    setChipSelect(devicePtr);

    isListening = true;

    if (!isCurrentlySending) {
        sendNullByteToSlave();
    }
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
    switch(UCA0IV)
    {
    // Receive data case
    case USCI_SPI_UCRXIFG: {

        // Check if receive handler has been set
        if (currentSlavePtr == NULL || currentSlavePtr->receiveHandler == NULL) {
            break;
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
        if (currentSlavePtr == NULL) {
            break;
        }

        if (slaveTransmitPtr == NULL) {
            // If we are listening, but not sending, send 0.
            if (isListening) {
                sendNullByteToSlave();
            } else {
                isCurrentlySending = false;
                setChipSelect(NULL);
            }
        } else {
            // If we have data to send, send it.
            sendByteToSlave();
        }

        break;
    }

    default:
        break;
    }
}
