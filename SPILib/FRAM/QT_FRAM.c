#include "QT_FRAM.h"
#include "SpiLib/QT_SPI_SpiLib.h"

#define FRAM_INDEX_START (FRAM_START + (uint16_t *) 8192)

static uint16_t *writeHead = FRAM_INDEX_START;
static uint16_t *readHead = FRAM_INDEX_START;

void QT_FRAM_initialise() {
    FRAMCtl_enableInterrupt(FRAMCTL_CORRECTABLE_BIT_INTERRUPT | FRAMCTL_UNCORRECTABLE_BIT_INTERRUPT);
}

/** Writes a single value to FRAM at writehead and increments the writehead. */
void QT_FRAM_write(uint16_t data) {
    FRAMCtl_write16(&data, writeHead++, 1);
}

/** Readys the FRAM buffer for writing. */
void QT_FRAM_reset() {
    writeHead = FRAM_INDEX_START;
    readHead = FRAM_INDEX_START;
}

/** Returns a pointer to the data. */
uint16_t QT_FRAM_dataPtr() {
    return FRAM_INDEX_START;
}

/** Gets the distance between the readhead and the writehead. */
int QT_FRAM_length() {
    return writeHead - FRAM_INDEX_START;
}
