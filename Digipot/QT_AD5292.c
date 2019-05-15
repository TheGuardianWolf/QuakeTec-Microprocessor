#include "QT_AD5292.h"

#include "../SpiLib/QT_MSP430_SPIMaster.h"
#include "../driverlib/MSP430FR2xx_4xx/driverlib.h"

#define RDAC_MAX 							(1023)

#define INSTRUCTION_NONE					(0x0000)
#define INSTRUCTION_WRITE_WIPER				(0x0400)
#define INSTRUCTION_READ_WIPER				(0x0800)
#define INSTRUCTION_STORE_WIPER				(0x0C00)
#define INSTRUCTION_RESET_WIPER				(0x1000)
#define INSTRUCTION_READ_20TP_MEMORY		(0x1400)
#define INSTRUCTION_WRITE_CONTROL_REGISTER	(0x1800)
#define INSTRUCTION_SWITCH_ON				(0x2000)
#define INSTRUCTION_SWITCH_OFF				(0x2001)

#define ERROR_NONE							(0)
#define ERROR_SPI_BUS						(1)
#define ERROR_INVALID_INPUT_VALUE			(2)

/* SPI configuration for the AD5292. */
static SPIMaster_Config_t t_Config =
{
	.b_ClockPhase = false,
	.b_ClockPolarity = false,
	.b_MSBFirst = true,
	.u8_CSPort = GPIO_PORT_P2,
	.u8_CSPin = GPIO_PIN6,
	.u16_ClockDivider = SPIMASTER_FREQUENCY_TO_DIVIDER(400000),
	.u16_CSDelay_us = 10,
	.u16_TransmissionDelay_us = 0,
};

void AD5292_Init()
{
    GPIO_setAsOutputPin(GPIO_PORT_P4, GPIO_PIN4);
    GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN4);
    GPIO_setAsOutputPin(t_Config.u8_CSPort, t_Config.u8_CSPin);
    GPIO_setOutputHighOnPin(t_Config.u8_CSPort, t_Config.u8_CSPin);
    __delay_cycles(100000);
}

uint8_t AD5292_SwitchOn()
{
	uint8_t u8_Status;
	uint16_t u16_TransferValue;

	/* Claim and configure SPI bus. */
	u8_Status = SPIMaster_StartTransfer(&t_Config);
	if (u8_Status != 0)
	{
		SPIMaster_EndTransfer(&t_Config);
		return ERROR_SPI_BUS;
	}

	/* Send switch-on instruction. */
	u16_TransferValue = INSTRUCTION_SWITCH_ON;
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

uint8_t AD5292_SwitchOff()
{
	uint8_t u8_Status;
	uint16_t u16_TransferValue;

	/* Claim and configure SPI bus. */
	u8_Status = SPIMaster_StartTransfer(&t_Config);
	if (u8_Status != 0)
	{
		SPIMaster_EndTransfer(&t_Config);
		return ERROR_SPI_BUS;
	}

	/* Send switch-off instruction. */
	u16_TransferValue = INSTRUCTION_SWITCH_OFF;
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

uint8_t AD5292_Set(uint16_t u16_Value)
{
	uint8_t u8_Status;
	uint16_t u16_TransferValue;

	if (u16_Value > RDAC_MAX) return ERROR_INVALID_INPUT_VALUE;

	/* Claim and configure SPI bus. */
	u8_Status = SPIMaster_StartTransfer(&t_Config);
	if (u8_Status != 0)
	{
		SPIMaster_EndTransfer(&t_Config);
		return ERROR_SPI_BUS;
	}

	/* Send enable wiper write instruction. */
	u16_TransferValue = INSTRUCTION_WRITE_CONTROL_REGISTER | 0b0010;
	u8_Status = SPIMaster_Transfer16(&u16_TransferValue);
	if (u8_Status != 0)
	{
		SPIMaster_EndTransfer(&t_Config);
		return ERROR_SPI_BUS;
	}

	/* Send wiper setting. */
	u16_TransferValue = INSTRUCTION_WRITE_WIPER | u16_Value;
	u8_Status = SPIMaster_Transfer16(&u16_TransferValue);
	if (u8_Status != 0)
	{
		SPIMaster_EndTransfer(&t_Config);
		return ERROR_SPI_BUS;
	}

	/* Send disable wiper write instruction. */
	u16_TransferValue = INSTRUCTION_WRITE_CONTROL_REGISTER | 0b0000;
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

uint8_t AD5292_Get(uint16_t *u16_Value)
{
	uint8_t u8_Status;

	/* Claim and configure SPI bus. */
	u8_Status = SPIMaster_StartTransfer(&t_Config);
	if (u8_Status != 0)
	{
		SPIMaster_EndTransfer(&t_Config);
		return ERROR_SPI_BUS;
	}

	/* Send read wiper setting instruction. */
	*u16_Value = INSTRUCTION_READ_WIPER;
	u8_Status = SPIMaster_Transfer16(u16_Value);
	if (u8_Status != 0)
	{
		SPIMaster_EndTransfer(&t_Config);
		return ERROR_SPI_BUS;
	}

	/* Send nothing and receive the wiper setting. */
	*u16_Value = INSTRUCTION_NONE;
	u8_Status = SPIMaster_Transfer16(u16_Value);
	if (u8_Status != 0)
	{
		SPIMaster_EndTransfer(&t_Config);
		return ERROR_SPI_BUS;
	}

	/* Release SPI bus. */
	SPIMaster_EndTransfer(&t_Config);

	return ERROR_NONE;
}

uint8_t AD5292_Store()
{
	uint8_t u8_Status;
	uint16_t u16_TransferValue;

	/* Claim and configure SPI bus. */
	u8_Status = SPIMaster_StartTransfer(&t_Config);
	if (u8_Status != 0)
	{
		SPIMaster_EndTransfer(&t_Config);
		return ERROR_SPI_BUS;
	}

	/* Send store instruction. */
	u16_TransferValue = INSTRUCTION_STORE_WIPER;
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

uint8_t AD5292_Reset()
{
	uint8_t u8_Status;
	uint16_t u16_TransferValue;

	/* Claim and configure SPI bus. */
	u8_Status = SPIMaster_StartTransfer(&t_Config);
	if (u8_Status != 0)
	{
		SPIMaster_EndTransfer(&t_Config);
		return ERROR_SPI_BUS;
	}

	/* Send reset instruction. */
	u16_TransferValue = INSTRUCTION_RESET_WIPER;
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
