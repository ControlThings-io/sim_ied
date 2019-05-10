#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
//#include "inc/hw_hibernate.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
//#include "driverlib/hibernate.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/systick.h"
#include "driverlib/uart.h"
#include "utils/ustdlib.h"
#include "utils/uartstdio.h"
#include "utils/cmdline.h"
#include "drivers/rgb.h"
#include "drivers/buttons.h"
#include "interface.h"
#include "aes.h"
#include "driverlib/i2c.h"

#include "eeprom_24lc08b.h"
#include "commands.h"

//*****************************************************************************
//
// Version Information
//
//*****************************************************************************

const char g_chVersion[] = "0.1";
const char g_chName[]    = "sim_ied";
const char g_chVendor[]  = "ControlThings I/O";
const char g_chWebsite[] = "https://controlthings.io";
const char g_chAuthor[]  = "Don C. Weber (cutaway)";
const char g_chContact[] = "don@cutawaysecurity.com";

//*****************************************************************************
//
// Input buffer for the command line interpreter.
//
//*****************************************************************************
static char g_cInput[APP_INPUT_BUF_SIZE];

//*****************************************************************************
//
// The error routine that is called if the driver library encounters an error.
//
//*****************************************************************************
#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
}
#endif


//*****************************************************************************
//
// Called by the NVIC as a result of SysTick Timer rollover interrupt flag
//
// Do something here.
//
//*****************************************************************************
void
SysTickIntHandler(void)
{
    /*
    Reference: https://codeblooms.com/how-to-use-systick-timer-in-tiva-c/
    */

}


//*****************************************************************************
//
// Configure the UART and its pins.  This must be called before UARTprintf().
//
//*****************************************************************************
void
ConfigureUART(void)
{
    //
    // Enable the GPIO Peripheral used by the UART.
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    //
    // Enable UART0
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    //
    // Configure GPIO Pins for UART mode.
    //
    ROM_GPIOPinConfigure(GPIO_PA0_U0RX);
    ROM_GPIOPinConfigure(GPIO_PA1_U0TX);
    ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    //
    // Use the internal 16MHz oscillator as the UART clock source.
    //
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);

    //
    // Initialize the UART for console I/O.
    //
    UARTStdioConfig(0, 115200, 16000000);
}

/**
 * main.c
 */
int main(void)
{
    //uint32_t ui32Status;
    //uint32_t ui32ResetCause;
    int32_t i32CommandStatus;

    //
    // Enable stacking for interrupt handlers.  This allows floating-point
    // instructions to be used within interrupt handlers, but at the expense of
    // extra stack usage.
    //
    ROM_FPUEnable();
    ROM_FPUStackingEnable();

    //
    // Set the system clock to run at 40Mhz off PLL with external crystal as
    // reference.
    //
    ROM_SysCtlClockSet(SYSCTL_SYSDIV_5 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ |
                       SYSCTL_OSC_MAIN);


    //
    // Enable and Initialize the UART.
    //
    ConfigureUART();
    //ConfigureI2C1();
    ROM_ConfigureI2C1();

    //
    // Setup EEPROM - this will test to see if the password has been written to
    //                to the EEPROM. If no password, then reset all EEPROM data.
    setEEPROMData();

    UARTprintf("\n\nControl Things IED Interface!\n");
    //UARTprintf("Type 'help' for a list of commands\n");
    //UARTprintf("> ");

    //
    // spin forever and wait for carriage returns or state changes.
    //
    while(1)
    {

        UARTprintf("\n> ");


        //
        // Peek to see if a full command is ready for processing
        //
        while(UARTPeek('\r') == -1)
        {
            //
            // millisecond delay.  A SysCtlSleep() here would also be OK.
            //
            SysCtlDelay(SysCtlClockGet() / (1000 / 3));

        }

        //
        // a '\r' was detected get the line of text from the user.
        //
        UARTgets(g_cInput,sizeof(g_cInput));

        //
        // Pass the line from the user to the command processor.
        // It will be parsed and valid commands executed.
        //
        i32CommandStatus = CmdLineProcess(g_cInput);

        //
        // Handle the case of bad command.
        //
        if(i32CommandStatus == CMDLINE_BAD_CMD)
        {
            UARTprintf("Bad command!\n");
        }

        //
        // Handle the case of too many arguments.
        //
        else if(i32CommandStatus == CMDLINE_TOO_MANY_ARGS)
        {
            UARTprintf("Too many arguments for command processor!\n");
        }

        //
        // Echo input back to user
        //
        //UARTprintf("Thank you for playing: ");
        //UARTprintf("%s\n",g_cInput);

    }

	return 0;
}
