#include "QT_MSP430_SPIMaster.h"
#include <msp430.h>
#include "../driverlib/MSP430FR2xx_4xx/driverlib.h"

/* The following pins are used for the SPI Master mode in eUSCI_B. */
#define PORT	GPIO_PORT_P4
#define MOSI	GPIO_PIN6
#define MISO	GPIO_PIN7
#define CLK	    GPIO_PIN5

#define REG_CTRL (0x05C0 + OFS_UCBxCTLW0)
#define REG_BITRATE (0x05C0 + OFS_UCBxBRW)
#define REG_STATUS (0x05C0 + OFS_UCBxSTATW)
#define REG_TX (0x05C0 + 0x000E)
#define REG_RX (0x05C0 + 0x000C)
#define REG_INTERRUPT_FLAGS (0x05C0 + 0x002C)

/* Stores a pointer to the configuration send to SPIMaster_StartTransfer. */
static SPIMaster_Config_t *pt_CurrentConfig;

void SPIMaster_Init()
{
    /* Configure MOSI and CLK. */
	GPIO_setAsPeripheralModuleFunctionOutputPin(
		PORT,
		MOSI | CLK,
		GPIO_PRIMARY_MODULE_FUNCTION
	);

	/* Configure MISO. */
	GPIO_setAsPeripheralModuleFunctionInputPin(
		PORT,
		MISO,
		GPIO_PRIMARY_MODULE_FUNCTION
	);
    /* Enable pull-up resistor for MISO. */
    P4DIR &= ~(BIT7);
    P4REN |= BIT7;
    P4OUT |= BIT7;

	/* Disable device during configuration. */
	HWREG16(REG_CTRL) |= UCSWRST;

	/* Clear control register (except UCSWRST bit). */
	HWREG16(REG_CTRL) &= UCSWRST;

	/* Configure control register as desired. */
	HWREG16(REG_CTRL) |= (
		0x0000
		//| UCCKPH	// Clock phase. We will set this manually for each device.
		//| UCCKPL	// Clock polarity. We will set this manually for each device.
		//| UCMSB	// MSB first. We will set this manually for each device.
		//| UC7BIT	// 7bit character length.
		| UCMST		// Master mode.
		//| UCMODE1	// eUSCI mode bit 1. We want 00 for 3-pin SPI mode.
		//| UCMODE0	// eUSCI mode bit 0.
		//| UCSYNC	// Synchronous mode.
		| UCSSEL1	// Clock source select bit 1. We're using SMCLK always.
		//| UCSSEL0	// Clock source select bit 0.
		//| UCSTEM	// STE mode select for master mode. Irrelevant in slave mode or 3 wire mode.
	);

	/* Clear bitrate control register. */
	HWREG16(REG_BITRATE) = 0x0000;

	/* Re-enable device. */
	HWREG16(REG_CTRL) &= ~(UCSWRST);
}

uint8_t SPIMaster_StartTransfer(SPIMaster_Config_t *pt_Config)
{
	/* Check if bus has already been reserved. */
	if (pt_CurrentConfig != NULL)
	{
		if (pt_CurrentConfig == pt_Config) return 1;	// Bus already reserved by this configuration.
		else return 2;									// Bus already reserved by a different configuration.
	}

	/* Store the input configuration. */
	pt_CurrentConfig = pt_Config;

	/* Disable device during configuration. */
	HWREG16(REG_CTRL) |= UCSWRST;

	/* Set clock phase. */
	if (pt_CurrentConfig->b_ClockPhase) HWREG16(REG_CTRL) |= UCCKPH;
	else HWREG16(REG_CTRL) &= ~(UCCKPH);

	/* Set clock polarity. */
	if (pt_CurrentConfig->b_ClockPolarity) HWREG16(REG_CTRL) |= UCCKPL;
	else HWREG16(REG_CTRL) &= ~(UCCKPL);

	/* Set MSB/LSB first. */
	if (pt_CurrentConfig->b_MSBFirst) HWREG16(REG_CTRL) |= UCMSB;
	else HWREG16(REG_CTRL) &= ~(UCMSB);

	/* Set clock divider. */
	HWREG16(REG_BITRATE) = pt_CurrentConfig->u16_ClockDivider;

	/* Re-enable device. */
	HWREG16(REG_CTRL) &= ~(UCSWRST);

	/* Indicate success. */
	return 0;
}

inline void Delay_us(uint32_t u32_us)
{
//    if (u16_us == 0) return;
//    float f = u16_us;
//    f /= 1000;
//    volatile struct timer* t1 = QT_sleep(f);
//    while (t1->command != TIMER_STOP)
//        ;

    /* Replaced interrupt timer with calibrated busy waiting for simplicity. Timer was crashing. */
    uint32_t u32_i;
    for (u32_i = 0; u32_i < CS_getMCLK() / (1000000*592) * u32_us; u32_i += 4)
        _delay_cycles(1);
}

uint8_t SPIMaster_Transfer8(uint8_t *u8_Data)
{
    if (pt_CurrentConfig == NULL) return 1;	// Bus not configured.

	/* Wait for any previous transfers to complete. */
	while (HWREG16(REG_STATUS) & UCBUSY)
		;

	/* Set CS low. */
	GPIO_setOutputLowOnPin(pt_CurrentConfig->u8_CSPort, pt_CurrentConfig->u8_CSPin);

	Delay_us(pt_CurrentConfig->u16_CSDelay_us);

	/* Write to Tx register. */
    HWREG16(REG_TX) = *u8_Data;

	/* Wait for data reception to complete. */
    while (!(HWREG16(REG_INTERRUPT_FLAGS) & UCRXIFG))
        ;

	/* Read received data from Rx register. */
	*u8_Data = (uint8_t) HWREG16(REG_RX);

	Delay_us(pt_CurrentConfig->u16_CSDelay_us);

	/* Set CS high. */
	GPIO_setOutputHighOnPin(pt_CurrentConfig->u8_CSPort, pt_CurrentConfig->u8_CSPin);

	Delay_us(pt_CurrentConfig->u16_TransmissionDelay_us);

	/* Indicate success. */
	return 0;
}

uint8_t SPIMaster_Transfer16(uint16_t *u16_Data)
{
	if (pt_CurrentConfig == NULL) return 1;	// Bus not configured.

	/* Split data into separate bytes */
	uint8_t u8_DataHigh = (uint8_t)(*u16_Data >> 8);
	uint8_t u8_DataLow = (uint8_t)(*u16_Data);

	/* Wait for any previous transfers to complete. */
	while (HWREG16(REG_STATUS) & UCBUSY)
		;

	/* Set CS low. */
	GPIO_setOutputLowOnPin(pt_CurrentConfig->u8_CSPort, pt_CurrentConfig->u8_CSPin);

	Delay_us(pt_CurrentConfig->u16_CSDelay_us);

	/* Write first byte to Tx register. */
	HWREG16(REG_TX) = u8_DataHigh;

	/* Wait for data transmission to complete. */
	while (!(HWREG16(REG_INTERRUPT_FLAGS) & UCRXIFG))
		;

	/* Read received data from Rx register. */
	u8_DataHigh = (uint8_t) HWREG16(REG_RX);

	/* Wait for bus to be ready again. */
	while (HWREG16(REG_STATUS) & UCBUSY)
		;

	/* Write second byte to Tx register. */
	HWREG16(REG_TX) = u8_DataLow;

	/* Wait for data reception to complete. */
	while (!(HWREG16(REG_INTERRUPT_FLAGS) & UCRXIFG))
		;

	/* Read received data from Rx register. */
	u8_DataLow = (uint8_t) HWREG16(REG_RX);

	Delay_us(pt_CurrentConfig->u16_CSDelay_us);

	/* Set CS high. */
	GPIO_setOutputHighOnPin(pt_CurrentConfig->u8_CSPort, pt_CurrentConfig->u8_CSPin);

	/* Reassemble data to 16 bit. */
	*u16_Data = (((uint16_t)u8_DataHigh) << 8) | u8_DataLow;

	Delay_us(pt_CurrentConfig->u16_TransmissionDelay_us);

	/* Indicate success. */
	return 0;
}

uint8_t SPIMaster_EndTransfer(SPIMaster_Config_t *pt_Config)
{
	if (pt_CurrentConfig == NULL) return 1;			// Bus was not reserved.
	if (pt_CurrentConfig != pt_Config) return 2;	// Incorrect configuration used to end transfer.

	/* Set CS high. */
	GPIO_setOutputHighOnPin(pt_CurrentConfig->u8_CSPort, pt_CurrentConfig->u8_CSPin);

	pt_CurrentConfig = NULL;

	return 0;
}

void SPIMaster_ForceEndTransfer()
{
	if (pt_CurrentConfig != NULL)
	{
		/* Set CS high. */
		GPIO_setOutputHighOnPin(pt_CurrentConfig->u8_CSPort, pt_CurrentConfig->u8_CSPin);
	}

	pt_CurrentConfig = NULL;
}
