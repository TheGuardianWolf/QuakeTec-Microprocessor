#include "QT_OBC_Interface.h"

#define EVENT_QUEUE_LENGTH 256
#define EVENT_QUEUE_HEADER_LENGTH 2

extern volatile uint16_t ERROR_CODE;
extern volatile bool exitCommand;

// Event buffer code. The buffer is on
byte eventQueue[EVENT_QUEUE_LENGTH + EVENT_QUEUE_HEADER_LENGTH];
int eventQueueStart = EVENT_QUEUE_LENGTH;
volatile bool finishedSendingEvents = true;
volatile f_flags F_FLAGS = {0};

// Sweep values
uint16_t dacValue;
uint8_t eventQueueLength=0;

uint8_t digipotControl;
uint8_t digipotData;
//byte asf[2]={0x00, 0x22};
byte empty[] = {0x00, 0x00};

void finishTransmission(bool unused) {
    QT_SPI_listenToMaster();
}

void flagEventsFinishedSending(bool unused) {
    finishedSendingEvents = true;

    finishTransmission(unused);
}

void sendQueryData(byte* data, uint8_t length) {
    byte headerData[2] = {length, PL_START_BYTE};

    finishedSendingEvents = false;

    QT_SPI_stopListeningToMaster(); // Stop reading until the data is sent

    QT_SPI_transmit(
            headerData,
            2,
            &OBC,
            flagEventsFinishedSending);

    while (!finishedSendingEvents && !exitCommand) {;}

    QT_SPI_stopListeningToMaster();

    QT_SPI_transmit(
            data,
            length,
            &OBC,
            flagEventsFinishedSending);

}

void queueEvent(PL_Event_t event) {
    if(eventQueueStart <= 0 || !finishedSendingEvents) {
        // Overflow. This should not occur given the OBC does not send more than 256 commands.
        return;
    }

    eventQueue[--eventQueueStart] = (byte) event;
}

void sendEvents() {
    eventQueue[eventQueueStart - 2] = 2;
    eventQueue[eventQueueStart - 1] = PL_START_BYTE;
    finishedSendingEvents = false;

    QT_SPI_stopListeningToMaster(); // Stop reading until the data is sent

    QT_SPI_transmit(
            &eventQueue[eventQueueStart - EVENT_QUEUE_HEADER_LENGTH],
            4,
            &OBC,
            flagEventsFinishedSending);


//    QT_SPI_transmit(
//            &asf, 2, &OBC, flagEventsFinishedSending
//            );

    eventQueueStart = EVENT_QUEUE_LENGTH;
}

byte queryData [4] = { PL_START_BYTE, 2, 0, 0 };

void handleQuery(PL_Query_t query, byte argument) {
    switch(query) {
    case PL_QUERY_EVENT:
        sendEvents();
        break;
    case PL_QUERY_ERROR_STATUS:
    {
        byte error_data[2] = {ERROR_STATUS >> 8, ERROR_STATUS & 0x00FF};
        sendQueryData(error_data, 2);
        break;
    }
    case PL_QUERY_DEPLOYMENT_STATE:
        sendQueryData(QT_BW_getDeploymentStatus(), 4);
        break;
    case PL_QUERY_PROBE_TEMPERATURE:
    {
        uint16_t temp = QT_IADC_readTemperature();
        byte temp_data[2] = {temp >> 8, temp & 0x00FF};
        sendQueryData(temp_data, 2);
    }

        break;
    case PL_QUERY_PROBE_DIGIPOT:
        sendQueryData(QT_DIGIPOT_getValue(), 2);
        break;
    case PL_QUERY_PROBE_FP:
        sendQueryData(QT_SW_getFloatingProbeVoltage(), 2);
        break;
    case PL_QUERY_DAC:
        sendQueryData(QT_DAC_getValue(), 2);
        break;
    case PL_QUERY_16V_POWER_CUR:
    {
        uint16_t temp = QT_IADC_readCurrent();
        byte temp_data[2] = {temp >> 8, temp & 0x00FF};
        sendQueryData(temp_data, 2);
    }
        break;
    case PL_QUERY_CONTACT_SWITCH:
        sendQueryData(QT_BW_getContactSwitchStatus(), 1);
        break;
    case PL_QUERY_SAMPLING_DATA:
        sendQueryData(QT_SW_getSweepData(), 2* SWEEP_MAX_NUM_SAMPLES * SWEEP_REPETITIONS);
        break;
    }
}

void handleCommand(PL_Command_t command, byte argument) {
    if(commandRunning && (command == PL_COMMAND_POWER_OFF || command == PL_COMMAND_STOP)) {
        exitCommand = true;
        queueEvent(PL_EVENT_TASK_INTERRUPTED);

        // Wait for the command to halt to ensure that the new value is read by the event loop
//        while(commandRunning);
    } else if (commandRunning) {
        return;
    }

    currentCommand = command;
    commandRunning = true;
}

void obcIncomingHandler(const byte *data) {
    byte code = data[0];
    byte argument = data[1];

    if (code == 0) {
        return;
    } else if ((code < PL_COMMAND_ENUM_COUNT)) {
        handleCommand((PL_Command_t) code, argument);//, data + 1);
    } else {
        handleQuery((PL_Query_t) code, argument);
    }
}

/**
 * Start the SPI communication listening lines.
 */
void startListening() {
    QT_SPI_setReceiveHandler(obcIncomingHandler, 2, &OBC);
    QT_SPI_listenToMaster();
}
