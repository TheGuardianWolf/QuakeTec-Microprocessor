#include "QT_OBC_Interface.h"
#include "SpiLib/QT_SPI_SpiLib.h"
#include <string.h>

#define EVENT_QUEUE_SIZE (255)

static uint16_t readBlock = 1;

static volatile PL_Command_t currentCommand = PL_COMMAND_IGNORE;
static volatile bool commandRunning = false;

// Event buffer code. The buffer is on
static PL_Event_t eventQueue[EVENT_QUEUE_SIZE];
static uint16_t eventQueueLength = 0;

static bool handleQuery(PL_Query_t query, uint8_t argument) {
    switch(query)
    {
        case PL_QUERY_EVENT:
        QT_OBCSPI_writeTx((uint8_t*)eventQueue, eventQueueLength);
        eventQueueLength = 0;
        return true;
        case PL_QUERY_ERROR_STATUS:
        {
            uint8_t data[2] = {(uint8_t)(ERROR_STATUS >> 8), (uint8_t)(ERROR_STATUS & 0xFF)};
            QT_OBCSPI_writeTx(data, 2);
        }
        return true;
        case PL_QUERY_DEPLOYMENT_STATE:
        QT_OBCSPI_writeTx(QT_BW_getDeploymentStatus(), 3);
        return true;
        case PL_QUERY_PROBE_POWER:
        {
            uint8_t data = (uint8_t)QT_PWR_getPowerStatus();
            QT_OBCSPI_writeTx(&data, 1);
        }
        return true;
        case PL_QUERY_PROBE_TEMPERATURE:
        {
            uint16_t temp = QT_IADC_readTemperature();
            uint8_t data[2] = {(uint8_t)(temp >> 8), (uint8_t)(temp & 0xFF)};
            QT_OBCSPI_writeTx(data, 2);
        }
        return true;
        case PL_QUERY_PROBE_DIGIPOT:
        QT_OBCSPI_writeTx(QT_DIGIPOT_getValue(), 2);
        return true;
        case PL_QUERY_PROBE_FP:
        QT_OBCSPI_writeTx(QT_SW_getFloatingProbeVoltage(), 2);
        return true;
        case PL_QUERY_DAC:
        QT_OBCSPI_writeTx(QT_DAC_getValue(), 2);
        return true;
        case PL_QUERY_16V_POWER_CUR:
        {
            uint16_t power = QT_IADC_readCurrent();
            uint8_t data[2] = {(uint8_t)(power >> 8), (uint8_t)(power & 0xFF)};
            QT_OBCSPI_writeTx(data, 2);
        }
        return true;
        case PL_QUERY_CONTACT_SWITCH:
        QT_OBCSPI_writeTx(QT_BW_getContactSwitchStatus(), 1);
        return true;
        case PL_QUERY_SAMPLING_DATA:
        QT_OBCSPI_writeTx(QT_SW_getSweepData(), 2* SWEEP_MAX_NUM_SAMPLES * SWEEP_REPETITIONS);
        return true;
        default:
        break;
    }
    return false;
}

static bool handleCommand(PL_Command_t command, byte argument) {
    if(commandRunning && (command == PL_COMMAND_POWER_OFF || command == PL_COMMAND_STOP)) {
        exitCommand = true;
        queueEvent(PL_EVENT_TASK_INTERRUPTED);
    } else if (commandRunning) {
        queueEvent(PL_EVENT_TASK_BUSY);
        return false;
    }

    currentCommand = command;
    commandRunning = true;
    return true;
}

static void rxDataHandler(uint8_t * const rxData, const uint16_t rxDataLength)
{
    if (readBlock == 1)
    {
        if (rxDataLength == 1 && rxData[0] == OBCSPI_FILL_BYTE)
        {
            QT_OBCSPI_clearRx();
        }
        else if (rxDataLength >= 2)
        {
            uint8_t code = rxData[0];
            uint8_t argument = rxData[1];
            bool block = false;

            if (code < PL_COMMAND_ENUM_COUNT)
            {
                (void)handleCommand((PL_Command_t) code, argument);
            }
            else
            {
                block = handleQuery((PL_Query_t) code, argument);
            }

            if (block)
            {
                readBlock = 0;
            }
            QT_OBCSPI_clearRx();
        }
    }
    else
    {
        if (rxDataLength >= 1 && rxData[0] == PL_INTERRUPT_BYTE)
        {
            QT_OBCSPI_clearTx();
            readBlock = 1;
        }
        QT_OBCSPI_clearRx();
    }
}

static void txDataHandler(uint8_t * const txData, const uint16_t txDataLength, const uint16_t txDataSent)
{
    if (readBlock == 0 && txDataSent == txDataLength)
    {
        QT_OBCSPI_clearTx();
        readBlock = 1;
    }
}

void QT_OBC_Interface_init()
{
    QT_OBCSPI_registerISRHandlers(&rxDataHandler, &txDataHandler);
}

void queueEvent(PL_Event_t event) {
    if (eventQueueLength < EVENT_QUEUE_SIZE)
    {
        eventQueue[eventQueueLength] = event;
        eventQueueLength += 1;
    }
}

void QT_OBC_Interface_commandLoop()
{
//         Wait until a command has been queued
        while(!commandRunning)
        {
            __no_operation();
        }
//         These command functions are blocking.
        switch(currentCommand) {
        case PL_COMMAND_STOP:
            break;
        case PL_COMMAND_POWER_OFF:
            QT_PWR_turnOff16V();
            queueEvent(PL_EVENT_PROBE_UNPOWERED);
            break;
        case PL_COMMAND_POWER_ON:
            QT_PWR_turnOn16V();
            queueEvent(PL_EVENT_PROBE_POWERED);
            break;
        case PL_COMMAND_SWEEP:
            queueEvent(PL_EVENT_SWEEP_STARTED);
            QT_SW_getPlasmaData();
            queueEvent(PL_EVENT_SWEEP_COMPLETE);
            break;
        case PL_COMMAND_DEPLOY:
            queueEvent(PL_EVENT_DEPLOY_STARTED);
            QT_BW_deploy();
            queueEvent(PL_EVENT_DEPLOY_COMPLETE);
            break;
        case PL_COMMAND_DEPLOY_SP:
            queueEvent(PL_EVENT_DEPLOY_SP_STARTED);
            QT_BW_deploySP();
            queueEvent(PL_EVENT_DEPLOY_SP_COMPLETE);
            break;
        case PL_COMMAND_DEPLOY_FP:
            queueEvent(PL_EVENT_DEPLOY_FP_STARTED);
            QT_BW_deployFP();
            queueEvent(PL_EVENT_DEPLOY_FP_COMPLETE);
            break;
        case PL_COMMAND_CLEAR_ERRORS:
            ERROR_STATUS = 0;
        default:
            break;
        }

        commandRunning = false;
}
