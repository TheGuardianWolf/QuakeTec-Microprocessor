#include "QT_ADC128S052.h"

#include "../SpiLib/QT_MSP430_SPIMaster.h"
#include "../driverlib/MSP430FR2xx_4xx/driverlib.h"

uint16_t u16_CurrentChannelMask = 0x0000;

/* SPI configuration for the ADC128S052. */
static SPIMaster_Config_t t_Config =
{
    .b_ClockPhase = false,
    .b_ClockPolarity = false,
    .b_MSBFirst = true,
    .u8_CSPort = GPIO_PORT_P2,
    .u8_CSPin = GPIO_PIN5,
    .u16_ClockDivider = SPIMASTER_FREQUENCY_TO_DIVIDER(100000),
    .u16_CSDelay_us = 10,
    .u16_TransmissionDelay_us = 0,
};

void ADC128S052_Init()
{
    GPIO_setAsOutputPin(t_Config.u8_CSPort, t_Config.u8_CSPin);
    GPIO_setOutputHighOnPin(t_Config.u8_CSPort, t_Config.u8_CSPin);
}

uint8_t ADC128S052_SelectChannel(uint8_t u8_Channel)
{
    uint8_t u8_Status;
    uint16_t u16_TransferValue;

    if (u8_Channel >= NUMBER_OF_CHANNELS) return ERROR_INVALID_CHANNEL;

    /* Claim and configure SPI bus. */
    u8_Status = SPIMaster_StartTransfer(&t_Config);
    if (u8_Status != 0)
    {
        SPIMaster_EndTransfer(&t_Config);
        return ERROR_SPI_BUS;
    }

    /* Send instruction containing channel id to use. */
    u16_CurrentChannelMask = ((uint16_t)u8_Channel) << (8 + 3);
    u16_TransferValue = u16_CurrentChannelMask;
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

uint8_t ADC128S052_Read(uint16_t *pu16_Value)
{
    uint8_t u8_Status;

    /* Claim and configure SPI bus. */
    u8_Status = SPIMaster_StartTransfer(&t_Config);
    if (u8_Status != 0)
    {
        SPIMaster_EndTransfer(&t_Config);
        return ERROR_SPI_BUS;
    }

    /* Send no instruction. */
    *pu16_Value = u16_CurrentChannelMask;
    u8_Status = SPIMaster_Transfer16(pu16_Value);
    if (u8_Status != 0)
    {
        SPIMaster_EndTransfer(&t_Config);
        return ERROR_SPI_BUS;
    }

    /* Release SPI bus. */
    SPIMaster_EndTransfer(&t_Config);

    return ERROR_NONE;
}
