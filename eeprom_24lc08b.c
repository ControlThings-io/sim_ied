/*
 * 24lc08b_eeprom.c
 *
 *  Created on: Apr 30, 2019
 *      Author: cutaway
 */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
//#include "inc/hw_ints.h"
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/systick.h"
#include "inc/hw_i2c.h"
#include "driverlib/i2c.h"

#include "eeprom_24lc08b.h"

#include "utils/ustdlib.h"
#include "utils/uartstdio.h"

//
// Something to store in firmware and eeprom. How about Base64 encoded encryption key: 16157e2ba6d2ae288815f7ab3c4fcf09
//
//const char g_chEncodedKey[] = "MTYxNTdlMmJhNmQyYWUyODg4MTVmN2FiM2M0ZmNmMDk=";


//*****************************************************************************
//
// Configure the I2C1 and its pins.  This must be called before using I2C.
// ROM version is preferred.
//
// As per: http://www.ti.com/lit/ug/spmu367/spmu367.pdf
// "The peripheral driver library can be utilized by applications to reduce their flash footprint,
// allowing the flash to be used for other purposes (such as additional features in the application)."
//
//*****************************************************************************
void ROM_ConfigureI2C1(void)
{

    //
    // Set up to communicate with Microchip 24LC08B EEPROM
    // using UART1. Requires a write-pin. Can set to GND to start
    // but need to update in the future to be a IO pin.
    //
    // Interface: I2C 2-Wire Serial Interface
    // Speed: 100 kHz (400 kHz Max)
    // Write Buffer: 16-byte Page
    // Blocks: 4 * 256 * 8-bit
    //
    // References:
    // http://ww1.microchip.com/downloads/en/devicedoc/21710k.pdf
    // https://energia.nu/pinmaps/img/EK-TM4C123GXL.jpg
    // https://e2e.ti.com/support/microcontrollers/other/f/908/t/416281
    // http://www.hobbytronics.co.uk/arduino-external-eeprom
    // https://stackoverflow.com/questions/24659919/i2c-interface-on-tiva/25808888
    // https://github.com/mahengunawardena/Tiva_I2C_Nokia_ADXL345/blob/master/Tiva_i2c.c
    // http://www.ti.com/lit/an/spma073/spma073.pdf

    //
    // Enable GPIO for I2C
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    while(!ROM_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOD));

    //
    // Configure GPIO Pins for I2C mode.
    //
    ROM_GPIOPinConfigure(GPIO_PD0_I2C3SCL);
    ROM_GPIOPinConfigure(GPIO_PD1_I2C3SDA);
    ROM_GPIOPinTypeI2CSCL(GPIO_PORTD_BASE, GPIO_PIN_0);
    ROM_GPIOPinTypeI2C(GPIO_PORTD_BASE, GPIO_PIN_1);

    //
    // Setup the system clock to run at 80 Mhz from PLL with crystal reference
    //
    //ROM_SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN);

    //
    // Reset and enable I2C
    ROM_SysCtlPeripheralDisable(SYSCTL_PERIPH_I2C3);
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C3);
    ROM_SysCtlPeripheralReset(SYSCTL_PERIPH_I2C3);
    while(!ROM_SysCtlPeripheralReady(SYSCTL_PERIPH_I2C3));

    //
    // Use the internal clock source: SysCtlClockGet for EK-TM4C123GLX.
    // 3rd parameter is speed: true is 400 kHz and false is 100 kHz
    //
    ROM_I2CMasterInitExpClk(I2C3_BASE, ROM_SysCtlClockGet(), false);

    //
    // Enable Interrupts for Arbitration Lost, Stop, NAK, Clock Low
    // Timeout and Data.
    //
    ROM_I2CMasterIntEnableEx(I2C3_BASE, (I2C_MASTER_INT_ARB_LOST |
    I2C_MASTER_INT_STOP | I2C_MASTER_INT_NACK |
    I2C_MASTER_INT_TIMEOUT | I2C_MASTER_INT_DATA));

    //
    // Enable interrupts to the processor.
    //
    ROM_IntMasterEnable();
    //ROM_IntEnable(INT_I2C3);

    //clear I2C FIFOs
    HWREG(I2C3_BASE + I2C_O_FIFOCTL) = 80008000;

}

//*****************************************************************************
//
// Configure the I2C1 and its pins.  This must be called before using I2C.
// This is the NON ROM version for testing.
//
//*****************************************************************************
/*
void ConfigureI2C1(void)
{

    //
    // Set up to communicate with Microchip 24LC08B EEPROM
    // using UART1. Requires a write-pin. Can set to GND to start
    // but need to update in the future to be a IO pin.
    //
    // Interface: I2C 2-Wire Serial Interface
    // Speed: 100 kHz (400 kHz Max)
    // Write Buffer: 16-byte Page
    // Blocks: 4 * 256 * 8-bit
    //
    // References:
    // http://www.ti.com/lit/ug/spmu367/spmu367.pdf
    // http://ww1.microchip.com/downloads/en/devicedoc/21710k.pdf
    // https://energia.nu/pinmaps/img/EK-TM4C123GXL.jpg
    // https://e2e.ti.com/support/microcontrollers/other/f/908/t/416281
    // http://www.hobbytronics.co.uk/arduino-external-eeprom
    // https://stackoverflow.com/questions/24659919/i2c-interface-on-tiva/25808888
    // https://github.com/mahengunawardena/Tiva_I2C_Nokia_ADXL345/blob/master/Tiva_i2c.c
    // http://www.ti.com/lit/an/spma073/spma073.pdf

    //
    // Enable GPIO for I2C
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOD));

    //
    // Configure GPIO Pins for I2C mode.
    //
    GPIOPinConfigure(GPIO_PD0_I2C3SCL);
    GPIOPinConfigure(GPIO_PD1_I2C3SDA);
    GPIOPinTypeI2CSCL(GPIO_PORTD_BASE, GPIO_PIN_0);
    GPIOPinTypeI2C(GPIO_PORTD_BASE, GPIO_PIN_1);

    //
    // Setup the system clock to run at 80 Mhz from PLL with crystal reference
    // This has already been set for the system during the UART config.
    //
    //SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN);

    //
    // Reset and enable I2C
    SysCtlPeripheralDisable(SYSCTL_PERIPH_I2C3);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C3);
    SysCtlPeripheralReset(SYSCTL_PERIPH_I2C3);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_I2C3));

    //
    // Use the internal clock source: SysCtlClockGet for EK-TM4C123GLX.
    // 3rd parameter is speed: true is 400 kHz and false is 100 kHz
    //
    I2CMasterInitExpClk(I2C3_BASE, SysCtlClockGet(), false);

    //
    // Enable Interrupts for Arbitration Lost, Stop, NAK, Clock Low
    // Timeout and Data.
    //
    I2CMasterIntEnableEx(I2C3_BASE, (I2C_MASTER_INT_ARB_LOST | I2C_MASTER_INT_STOP | I2C_MASTER_INT_NACK | I2C_MASTER_INT_TIMEOUT | I2C_MASTER_INT_DATA));

    //
    // Enable interrupts to the processor.
    //
    IntMasterEnable();


}
*/

void eepromRead(uint32_t dev_addr, uint16_t address_u16, uint32_t *rxdata_pu8, uint32_t rxdataLen_u32)
{
    int i;
    //UARTprintf("\nEEPROM Data for Slave 0x%X from page %d, address %d: \n",ADDRESS_24LC08B,0,address_u16);

    // Setup and move pointer to the address location.
    // Buspirate: [0xa0 address_u16]
    ROM_I2CMasterSlaveAddrSet(I2C3_BASE, ADDRESS_24LC08B, false);
    ROM_I2CMasterDataPut(I2C3_BASE, address_u16);    //READ
    ROM_I2CMasterControl(I2C3_BASE, I2C_MASTER_CMD_SINGLE_SEND);
    while(ROM_I2CMasterBusy(I2C3_BASE)){}

    // Start reading
    ROM_I2CMasterSlaveAddrSet(I2C3_BASE, dev_addr, true);
    ROM_I2CMasterControl(I2C3_BASE, I2C_MASTER_CMD_BURST_RECEIVE_START );
    while(ROM_I2CMasterBusy(I2C3_BASE)){}
    rxdata_pu8[0] = ROM_I2CMasterDataGet(I2C3_BASE) & 0xFF;

    // NOTE: Reads are not affected by the 16 byte write buffer.
    //       The EEPROM's incrementer will go to end of page.
    //       Incrementer probably wraps at end of page, need to test.
    for (i=1;i<rxdataLen_u32;i++){
        ROM_I2CMasterControl(I2C3_BASE, I2C_MASTER_CMD_BURST_RECEIVE_CONT );
        while(ROM_I2CMasterBusy(I2C3_BASE)){}
        rxdata_pu8[i] = ROM_I2CMasterDataGet(I2C3_BASE) & 0xFF;
    }

    // NOTE: need to test if we get an extra byte, like we handled via the write.
    //       I don't believe we do.
    ROM_I2CMasterControl(I2C3_BASE, I2C_MASTER_CMD_BURST_RECEIVE_FINISH );
    while(ROM_I2CMasterBusy(I2C3_BASE)){}
    rxdata_pu8[i] = ROM_I2CMasterDataGet(I2C3_BASE) & 0xFF;

}

void eepromWrite(uint32_t dev_addr, uint16_t address_u16, uint8_t *txdata_pu8, uint32_t txdataLen_u32)
{
    // NOTE: This function does not manage delay.
    //       Not having a good delay between writes will
    //       result in NACKs instead of ACKs. ACKs are
    //       required for BURST send. Watch your writes
    //       using a logic analyzer and tune appropriately.
    int i;

    /*
    // Setup and move pointer to the address location.
    // Buspirate: [0xa0 address_u16]
    ROM_I2CMasterSlaveAddrSet(I2C3_BASE, ADDRESS_24LC08B, false);
    ROM_I2CMasterDataPut(I2C3_BASE, address_u16);    //READ
    ROM_I2CMasterControl(I2C3_BASE, I2C_MASTER_CMD_SINGLE_SEND);
    while(ROM_I2CMasterBusy(I2C3_BASE)){}
    ROM_SysCtlDelay( (ROM_SysCtlClockGet() / 3) / 100 );
    */

    // Setup and move pointer to the address location.
    ROM_I2CMasterSlaveAddrSet(I2C3_BASE, dev_addr, false);
    ROM_I2CMasterDataPut(I2C3_BASE,address_u16);
    // Start the burst. Works even for just one byte due to the for loop
    ROM_I2CMasterControl(I2C3_BASE, I2C_MASTER_CMD_BURST_SEND_START );
    while(ROM_I2CMasterBusy(I2C3_BASE)){}

    // Loop thru the data
    // NOTE: If txdataLen_u32 is > 16 the write buffer will loop.
    //       This function does not manage > 16 bytes of data
    //       and should be feed only 16 bytes at a time.
    for (i=0;i<txdataLen_u32;i++){
        ROM_I2CMasterDataPut(I2C3_BASE,txdata_pu8[i]);
        // Test for last char and use FINISH to avoid sending last char twice
        if (txdataLen_u32 - (i + 1)){
            ROM_I2CMasterControl(I2C3_BASE, I2C_MASTER_CMD_BURST_SEND_CONT );
        } else {
            ROM_I2CMasterControl(I2C3_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH );
        }
        while(ROM_I2CMasterBusy(I2C3_BASE)){}
    }
    // Quick delay just to let EEPROM write settle.
    // Not long enough for multiple writes.
    ROM_SysCtlDelay(10);

}


void resetPtrEEPROM()
{
    // Setup and move pointer to the address location.
    // Buspirate: [0xa0 address_u16]
    ROM_I2CMasterSlaveAddrSet(I2C3_BASE, ADDRESS_24LC08B, false);
    ROM_I2CMasterDataPut(I2C3_BASE, 0x0);    //READ
    ROM_I2CMasterControl(I2C3_BASE, I2C_MASTER_CMD_SINGLE_SEND);
    while(ROM_I2CMasterBusy(I2C3_BASE)){}
    ROM_SysCtlDelay( (ROM_SysCtlClockGet() / 3) / 100 );
}
