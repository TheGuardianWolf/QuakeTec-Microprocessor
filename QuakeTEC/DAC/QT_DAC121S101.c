#include "QT_DAC121S101.h"

#include "../SpiLib/QT_MSP430_SPIMaster.h"
#include "../driverlib/MSP430FR2xx_4xx/driverlib.h"

#define DAC_MAX                 (0x0FFF)
#define MODE_MAX                (3)

/* The following variables store the last setting written to the digipot,
 * so that new settings can be written to either the value or mode without
 * resetting the other. */
static uint16_t u16_ModeMask = 0x0000;
static uint16_t u16_ValueMask = 0x0000;

/* SPI configuration for the DAC121S101. */
static SPIMaster_Config_t t_Config =
{
    .b_ClockPhase = false,
    .b_ClockPolarity = false,
    .b_MSBFirst = true,
    .u8_CSPort = GPIO_PORT_P2,
    .u8_CSPin = GPIO_PIN4,
    .u16_ClockDivider = SPIMASTER_FREQUENCY_TO_DIVIDER(4000000),
    .u16_CSDelay_us = 10,
    .u16_TransmissionDelay_us = 0,
};

void DAC121S101_Init()
{
    GPIO_setAsOutputPin(t_Config.u8_CSPort, t_Config.u8_CSPin);
    GPIO_setOutputHighOnPin(t_Config.u8_CSPort, t_Config.u8_CSPin);
}

uint8_t DAC121S101_Set(uint16_t u16_Value)
{
    uint8_t u8_Status;
    uint16_t u16_TransferValue;

    if (u16_Value > DAC_MAX) return ERROR_INPUT_TOO_HIGH;

    /* Claim and configure SPI bus. */
    u8_Status = SPIMaster_StartTransfer(&t_Config);
    if (u8_Status != 0)
    {
        SPIMaster_EndTransfer(&t_Config);
        return ERROR_SPI_BUS;
    }

    /* Set value mask. */
    u16_ValueMask = u16_Value;

    /* Send instruction containing the mode setting and value. */
    u16_TransferValue = u16_ModeMask | u16_ValueMask;
    u8_Status = SPIMaster_Transfer16(&u16_TransferValue);
//    if (u8_Status != 0)
//    {
//        SPIMaster_EndTransfer(&t_Config);
//        return ERROR_SPI_BUS;
//    }

    /* Release SPI bus. */
    SPIMaster_EndTransfer(&t_Config);

    return ERROR_NONE;
}

uint8_t DAC121S101_SetMode(uint8_t u8_Mode)
{
    uint8_t u8_Status;
    uint16_t u16_TransferValue;

    if (u8_Mode > MODE_MAX) return ERROR_MODE_INVALID;

    /* Claim and configure SPI bus. */
    u8_Status = SPIMaster_StartTransfer(&t_Config);
    if (u8_Status != 0)
    {
        SPIMaster_EndTransfer(&t_Config);
        return ERROR_SPI_BUS;
    }

    /* Set mode mask. */
    u16_ModeMask = ((uint16_t)u8_Mode) << 12;

    /* Send instruction containing the mode setting and value. */
    u16_TransferValue = u16_ModeMask | u16_ValueMask;
    u8_Status = SPIMaster_Transfer16(&u16_TransferValue);
    if (u8_Status != 0)
    {
        SPIMaster_EndTransfer(&t_Config);
        return ERROR_SPI_BUS;
    }

    /* Release SPI bus. */
    SPIMaster_EndTransfer(&t_Config);

    return ERROR_NONE;
}
