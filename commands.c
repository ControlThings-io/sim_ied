/*
 * commands.c
 *
 *  Created on: Apr 21, 2019
 *      Author: cutaway
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "inc/hw_types.h"
#include "utils/ustdlib.h"
#include "utils/uartstdio.h"
#include "utils/cmdline.h"
#include "interface.h"
#include "commands.h"
#include "aes.h"
#include "crypto.h"
#include "eeprom_24lc08b.h"
#include "driverlib/rom.h"

//*****************************************************************************
//
// Table of valid command strings, callback functions and help messages.  This
// is used by the cmdline module.
//
//*****************************************************************************
tCmdLineEntry g_psCmdTable[] =
{
    {"help",     CMD_help,      " : Display list of commands" },
    {"encbytes", CMD_encbytes,  " : String of bytes to Encrypt. Input must be divisible by 2. Example: deadbeef"},
    {"decbytes", CMD_decbytes,  " : String of bytes to Decrypt. Input must be divisible by 16. Example: 000102030405060708090a0b0c0d0e0f"},
    {"getkey",   CMD_gkey,      " : Get current encryption key. Default: 000102030405060708090a0b0c0d0e0f"},
    {"setkey",   CMD_skey,      " : Set encryption key. Key must be 128-Bits which is 16 bytes. See default value."},
    {"dumpdata", CMD_dumpdata,  " : Get data from the external EEPROM. Prints four blocks of 256 bytes in hex format."},
    {"getdata",  CMD_getdata,   " : Get data from the external EEPROM. Requires page, starting position, and count. Example: getdata 0 0 32"},
    {"setdata",  CMD_setdata,   " : Set lab data on the external EEPROM. "},
    {"clrdata",  CMD_clrdata,   " : Clear all data on external EEPROM. Requires password. Example: clrdata <pass>"},
    {"clrpage",  CMD_clrpage,   " : Clear a page of data on external EEPROM. Requires password. Input must be 0-3. Example: clrpage <pass> 2"},
    //{"testpass", CMD_testpass,  " : Test if use provided password."},
    { 0, 0, 0 }
};

//*****************************************************************************
//
// Helper Functions
//
//*****************************************************************************

bool testPasswd(char *indata_ch32){

    int i;
    bool success = false;
    uint8_t maxPasswd = 32;
    uint32_t ui32Result[33];
    uint8_t ui8Result[33];
    uint32_t ui32Page = 2;
    uint16_t ui16Start = 0;
    uint32_t ui32Cnt = maxPasswd;
    uint32_t ui32UsrAddr = ADDRESS_24LC08B + ui32Page;

    //
    // DEBUG: use "admin" for a password during testing.
    // DEBUG: remove for production.
    //
    if (!(strncmp("admin",indata_ch32,maxPasswd))){
        success = true;
        return success;
    }

    //
    // Need to convert from uint32_t to uint3_t so that
    // it can be converted to a (char *) and tested.
    //
    eepromRead(ui32UsrAddr, ui16Start, ui32Result, ui32Cnt);
    for (i=0;i<(maxPasswd + 1);i++){

        ui8Result[i] = ui32Result[i];
    }

    if (!(strncmp((char *) ui8Result,indata_ch32,maxPasswd))){
        success = true;
    }
    return success;

}

int writeString(uint32_t page_u32, uint16_t address_u16, char *txdata_ch8, uint32_t txdataLen_u32)
{

    int i, passes = 0;
    uint16_t ui16Start = address_u16;
    // Include the /0 at end of string
    uint32_t ui32Cnt = txdataLen_u32 + 1;
    uint32_t ui32CntExtra = 0;
    uint32_t ui32UsrAddr = ADDRESS_24LC08B + page_u32;

    passes = (ui32Cnt / 16) + 1;
    ui32CntExtra = txdataLen_u32 % 16;
    for (i=0;i<passes;i++){
        if ((i + 1) == passes){
            eepromWrite(ui32UsrAddr, ui16Start + (i * 16), txdata_ch8 + (i*16), ui32CntExtra);
        } else {
            eepromWrite(ui32UsrAddr, ui16Start + (i * 16), txdata_ch8 + (i*16), 16);
        }
        // This delay seems to work best for writing
        // https://gist.github.com/ctring/7f12d812fb594eecc493
        ROM_SysCtlDelay( (ROM_SysCtlClockGet() / 3) / 100 );
    }

    // Return int
    return(0);
}

void setEEPROMData()
{

    uint32_t ui32Page;
    uint16_t ui16Start;
    uint32_t ui32Cnt;
    uint8_t ui8Padding[16];
    memset(ui8Padding,0xff,16);

    ///////////////////////
    // DATA FOR STORAGE
    ///////////////////////
    // Hex of password: 537570657223536563726574235061737377307264
    char password[] = "Super#Secret#Passw0rd";
    // Hex of password: 4c657427732347657423493243234f6e2350617373
    //char password[] = "Let's#Get#I2C#On#Pass";
    //
    // The following is included for clarity. See crypto.c
    //
    // Encryption key
    //
    //char g_chAES128Key[] = "16157e2ba6d2ae288815f7ab3c4fcf09";
    //uint8_t g_ui8AES128Key[16] = {};
    // Base64 Encoded key is a global const char g_chEncodedKey
    //char key_base64[] = "MTYxNTdlMmJhNmQyYWUyODg4MTVmN2FiM2M0ZmNmMDk=";
    ///////////////////////

    /************************************
    * PAGE 0 DATA
    * Write the vendor / device data
    *
    * const char g_chVersion[] = "0.1";
    * const char g_chName[]    = "sim_ied";
    * const char g_chVendor[]  = "ControlThings I/O";
    * const char g_chWebsite[] = "https://controlthings.io";
    * const char g_chAuthor[]  = "Don C. Weber (cutaway)";
    * const char g_chContact[] = "don@cutawaysecurity.com";
    *************************************/
    ui32Page = 0;
    ui16Start = 0;
    //ui32UsrAddr = ADDRESS_24LC08B + ui32Page;

    // FIXME: These writes are broken.
    //        The incrementer does not reset after a I2C burst.
    //        Even a read operation after the write does
    //        not reset the EEPROM's incrementer. Thus the
    //        vendor information needs to be lumped together
    //        and written in a loop. Best option would be
    //        to determine how to reset the incrementer.

    resetPtrEEPROM();
    ui32Cnt = strlen(g_chVersion) + 1;
    writeString(ui32Page, ui16Start, g_chVersion, ui32Cnt);
    // This delay seems to work best for writing
    // https://gist.github.com/ctring/7f12d812fb594eecc493
    ROM_SysCtlDelay( (ROM_SysCtlClockGet() / 3) / 100 );
    // FIXME: There HAS to be a better way.
    // NOTE: The following is necessary to fill the remaining
    //       16-byte section ("page") due to issues with
    //       the incrementer not clearing after successive
    //       writes.
    ui16Start += ui32Cnt;
    ui32Cnt = (16-(ui32Cnt % 16));
    writeString(ui32Page, ui16Start, ui8Padding, ui32Cnt);
    ROM_SysCtlDelay( (ROM_SysCtlClockGet() / 3) / 100 );

    resetPtrEEPROM();
    ui16Start += ui32Cnt;
    ui32Cnt = strlen(g_chName) + 1;
    writeString(ui32Page, ui16Start, g_chName, ui32Cnt);
    // This delay seems to work best for writing
    // https://gist.github.com/ctring/7f12d812fb594eecc493
    ROM_SysCtlDelay( (ROM_SysCtlClockGet() / 3) / 100 );
    // FIXME: There HAS to be a better way.
    // NOTE: The following is necessary to fill the remaining
    //       16-byte section ("page") due to issues with
    //       the incrementer not clearing after successive
    ui16Start += ui32Cnt;
    ui32Cnt = (16-(ui32Cnt % 16));
    writeString(ui32Page, ui16Start, ui8Padding, ui32Cnt);
    ROM_SysCtlDelay( (ROM_SysCtlClockGet() / 3) / 100 );

    resetPtrEEPROM();
    ui16Start += ui32Cnt;
    ui32Cnt = strlen(g_chVendor) + 1;
    writeString(ui32Page, ui16Start, g_chVendor, ui32Cnt);
    // This delay seems to work best for writing
    // https://gist.github.com/ctring/7f12d812fb594eecc493
    ROM_SysCtlDelay( (ROM_SysCtlClockGet() / 3) / 100 );
    // FIXME: There HAS to be a better way.
    // NOTE: The following is necessary to fill the remaining
    //       16-byte section ("page") due to issues with
    //       the incrementer not clearing after successive
    ui16Start += ui32Cnt;
    ui32Cnt = (16-(ui32Cnt % 16));
    writeString(ui32Page, ui16Start, ui8Padding, ui32Cnt);
    ROM_SysCtlDelay( (ROM_SysCtlClockGet() / 3) / 100 );

    ui16Start += ui32Cnt;
    ui32Cnt = strlen(g_chWebsite) + 1;
    writeString(ui32Page, ui16Start, g_chWebsite, ui32Cnt);
    // FIXME: There HAS to be a better way.
    // NOTE: The following is necessary to fill the remaining
    //       16-byte section ("page") due to issues with
    //       the incrementer not clearing after successive
    ui16Start += ui32Cnt;
    ui32Cnt = (16-(ui32Cnt % 16));
    writeString(ui32Page, ui16Start, ui8Padding, ui32Cnt);
    ROM_SysCtlDelay( (ROM_SysCtlClockGet() / 3) / 100 );

    ui16Start += ui32Cnt;
    ui32Cnt = strlen(g_chAuthor) + 1;
    writeString(ui32Page, ui16Start, g_chAuthor, ui32Cnt);
    // FIXME: There HAS to be a better way.
    // NOTE: The following is necessary to fill the remaining
    //       16-byte section ("page") due to issues with
    //       the incrementer not clearing after successive
    ui16Start += ui32Cnt;
    ui32Cnt = (16-(ui32Cnt % 16));
    writeString(ui32Page, ui16Start, ui8Padding, ui32Cnt);
    ROM_SysCtlDelay( (ROM_SysCtlClockGet() / 3) / 100 );

    ui16Start += ui32Cnt;
    ui32Cnt = strlen(g_chContact) + 1;
    writeString(ui32Page, ui16Start, g_chContact, ui32Cnt);
    // FIXME: There HAS to be a better way.
    // NOTE: The following is necessary to fill the remaining
    //       16-byte section ("page") due to issues with
    //       the incrementer not clearing after successive
    ui16Start += ui32Cnt;
    ui32Cnt = (16-(ui32Cnt % 16));
    writeString(ui32Page, ui16Start, ui8Padding, ui32Cnt);
    ROM_SysCtlDelay( (ROM_SysCtlClockGet() / 3) / 100 );

    /************************************
    * PAGE 2 DATA
    * Write the password to Page 2 at offset 0
    *************************************/
    ui32Page = 2;
    ui16Start = 0;
    //ui32UsrAddr = ADDRESS_24LC08B + ui32Page;
    // Include the /0 at end of string
    ui32Cnt = strlen(password) + 1;

    writeString(ui32Page, ui16Start, password, ui32Cnt);

    /************************************
    * PAGE 3 DATA
    * Write the BASE64 key and the actual key at offset 0
    *************************************/
    ui32Page = 3;
    ui16Start = 0;
    //ui32UsrAddr = ADDRESS_24LC08B + ui32Page;
    // Include the /0 at end of string
    ui32Cnt = strlen(g_chEncodedKey) + 1;

    writeString(ui32Page, ui16Start, g_chEncodedKey, ui32Cnt);
    // FIXME: There HAS to be a better way.
    // NOTE: The following is necessary to fill the remaining
    //       16-byte section ("page") due to issues with
    //       the incrementer not clearing after successive
    ui16Start += ui32Cnt;
    ui32Cnt = (16-(ui32Cnt % 16));
    writeString(ui32Page, ui16Start, ui8Padding, ui32Cnt);
    ROM_SysCtlDelay( (ROM_SysCtlClockGet() / 3) / 100 );

    // Write the ASCII value of the key.
    ui16Start += ui32Cnt;
    ui32Cnt = strlen(g_chAES128Key) + 1;
    writeString(ui32Page, ui16Start, g_chAES128Key, ui32Cnt);
    // FIXME: There HAS to be a better way.
    // NOTE: The following is necessary to fill the remaining
    //       16-byte section ("page") due to issues with
    //       the incrementer not clearing after successive
    ui16Start += ui32Cnt;
    ui32Cnt = (16-(ui32Cnt % 16));
    writeString(ui32Page, ui16Start, ui8Padding, ui32Cnt);
    ROM_SysCtlDelay( (ROM_SysCtlClockGet() / 3) / 100 );

    // Write the binary of the key
    ui16Start += ui32Cnt;
    ui32Cnt = (strlen(g_chAES128Key) + 1)/2;
    set_value(&g_chAES128Key, &g_ui8AES128Key);
    writeString(ui32Page, ui16Start, g_ui8AES128Key, ui32Cnt);
    // FIXME: There HAS to be a better way.
    // NOTE: The following is necessary to fill the remaining
    //       16-byte section ("page") due to issues with
    //       the incrementer not clearing after successive
    ui16Start += ui32Cnt;
    ui32Cnt = (16-(ui32Cnt % 16));
    writeString(ui32Page, ui16Start, ui8Padding, ui32Cnt);
    ROM_SysCtlDelay( (ROM_SysCtlClockGet() / 3) / 100 );

}

//*****************************************************************************
//
// Command: help
//
// Print the help strings for all commands.
//
//*****************************************************************************
int CMD_help(int argc, char **argv)
{
    int32_t i32Index;

    (void)argc;
    (void)argv;

    //
    // Start at the beginning of the command table
    //
    i32Index = 0;

    //
    // Get to the start of a clean line on the serial output.
    //
    UARTprintf("\nAvailable Commands\n------------------\n\n");

    //
    // Display strings until we run out of them.
    //
    while(g_psCmdTable[i32Index].pcCmd)
    {
      //UARTprintf("%17s %s\n", g_psCmdTable[i32Index].pcCmd, g_psCmdTable[i32Index].pcHelp);
      UARTprintf("%10s %s\n", g_psCmdTable[i32Index].pcCmd, g_psCmdTable[i32Index].pcHelp);
      i32Index++;
    }

    //
    // Leave a blank line after the help strings.
    //
    UARTprintf("\n");

    return (0);
}

//*****************************************************************************
//
// Command: Encrypt Data
//
// Takes a single argument that is a string to be encrypted. The argument
// must be an string that is divisible by 2.  The data is encrypted and the
// encrypted data is returned.
//
//*****************************************************************************
int CMD_encbytes(int argc, char **argv)
{


    size_t arrlen;
    char result[MAX_STR + 1];
    memset(result,0x00,MAX_STR +1);

    //
    // Initialize AES if necessary
    //
    if (!(GET_aes())){
        SET_aes();
    }


    //
    // This command requires one parameter.
    //
    if(argc != 2){
        UARTprintf("encbytes Command Error. Type help for assistance.\n");
        return(1);
    }

    arrlen = strlen(argv[1]);

    if ((arrlen > MAX_STR) || (arrlen % 2)){
        UARTprintf("encbytes Input Error. Type help for assistance.\n");
        return(1);
    }

    //
    // Extract the plain text from the command line parameter.
    //

    strncpy(&g_chAESPlainText,argv[1],MAX_STR);

    set_value(&g_chAESPlainText, &g_ui8AESPlainText);

    //UARTprintf("Encrypting Input.\n");

    //
    // Encrypt the data.
    //
    enc_blocks(result,&g_ui8AESPlainText, arrlen/2);
    //UARTprintf("Encrypt Result: %s\n",result);
    UARTprintf("%s\n",result);

    //
    // Leave a blank line after the strings.
    //
    UARTprintf("\n");

    // Return int
    return(0);

}

//*****************************************************************************
//
// Command: Decrypt Data
//
// Takes a single argument that is a string to be decrypted. The argument
// must be a string that is divisible by 16.  The data is decrypted and the
// plain text data is returned.
//
//*****************************************************************************
int CMD_decbytes(int argc, char **argv)
{

    size_t arrlen;
    char result[MAX_STR + 1];
    memset(result,0x00,MAX_STR + 1);

    //
    // Initialize AES if necessary
    //
    if (!(GET_aes())){
        SET_aes();
    }

    //
    // This command requires one parameter.
    //
    if(argc != 2){
        UARTprintf("decbytes Command Error. Type help for assistance.\n");
        return(1);
    }

    arrlen = strlen(argv[1]);
    if ((arrlen > MAX_STR) || (arrlen % 16)){
        UARTprintf("decbytes Input Error. Type help for assistance.\n");
        return(1);
    }
    //
    // Extract the plain text from the command line parameter.
    //
    strncpy(&g_chAESCipherText,argv[1],arrlen);

    set_value(&g_chAESCipherText, &g_ui8AESCipherText);

    //
    // Decrypt the data.
    //
    dec_blocks(result,&g_ui8AESCipherText, arrlen/2);
    //UARTprintf("Encrypt Result: %s\n",result);
    UARTprintf("%s\n",result);

    UARTprintf("\n");

    // Return int
    return(0);

}

//*****************************************************************************
//
// Command: Get Encryption Key
//
// Takes no arguments.
//
//*****************************************************************************
int CMD_gkey(int argc, char **argv)
{

    if(argc != 1){
        UARTprintf("getkey Command Error. Type help for assistance.\n");
        return(1);
    }

    UARTprintf("Current Encryption Key: %s",&g_chAES128Key);
    UARTprintf("\n");

    // Return int
    return(0);

}


//*****************************************************************************
//
// Command: Set Encryption Key
//
// Takes a single argument that is the encryption key to be used. The argument
// must be a 32-byte string, ASCII representation of the 128-bit key.
//
//*****************************************************************************
int CMD_skey(int argc, char **argv)
{

    size_t arrlen;

    //
    // This command requires one parameter.
    //
    if(argc != 2){
        UARTprintf("setkey Command Error. Type help for assistance.\n");
        return(1);
    }

    // Check the size of the incoming key.
    arrlen = strlen(argv[1]);
    if (arrlen != 32){
        UARTprintf("setkey Input Error. Type help for assistance.\n");
        return(1);
    }

    // Store new key.
    strncpy(&g_chAES128Key,argv[1],arrlen);
    set_value(&g_chAES128Key, &g_ui8AES128Key);

    // Reset AES because we have updated the key.
    RESET_aes();

    //
    // Leave a blank line after the strings.
    //
    UARTprintf("Key set to: %s\n",&g_chAES128Key);

    // Return int
    return(0);

}

//*****************************************************************************
//
// Command: Dump all data from external EEPROM
//
// Takes no arguments.
//
//*****************************************************************************
int CMD_dumpdata(int argc, char **argv)
{
    int i;
    uint32_t ui32Result[256];
    uint32_t ui32Page = 0;
    uint32_t ui32Start = 0;
    uint32_t ui32Cnt = 256;
    uint32_t ui32UsrAddr;
    //char result[257];

    //
    // Loop thru each page and print contents
    for (ui32Page = 0; ui32Page < 4; ui32Page++){
        //
        // Update device address with the page that the user provided
        //
        //memset(result,0x00,257);
        memset(ui32Result,0x00,256);
        ui32UsrAddr = ADDRESS_24LC08B + ui32Page;
        eepromRead(ui32UsrAddr, ui32Start, ui32Result, ui32Cnt);

        UARTprintf("Page: %d\n",ui32Page);
        for (i=0;i<ui32Cnt;i++){
            UARTprintf("%02x",ui32Result[i]);
            //result[i] = ui32Result[i];
        }
        UARTprintf("\n");

        //UARTprintf("String: %s",result);
        //UARTprintf("\n");
    }

    // Return int
    return(0);

}

//*****************************************************************************
//
// Command: Get data from external EEPROM
//
// Takes three arguments
//     page - the block number to read from which ranges from 0 to 3.
//     start - the starting address to read from which ranges from 0 to 255.
//     count - the number of bytes to read.
//
//*****************************************************************************
int CMD_getdata(int argc, char **argv)
{
    int i;
    uint32_t ui32Result[256];
    uint32_t ui32Page;
    uint32_t ui32Start;
    uint32_t ui32Cnt;
    uint32_t ui32UsrAddr;

    //
    // This command requires one parameter.
    //
    if(argc != 4){
        UARTprintf("getdata Command Error. Type help for assistance.\n");
        return(1);
    }

    //
    // Extract the intensity from the command line parameter.
    //
    ui32Page  = ustrtoul(argv[1], 0, 10);
    ui32Start = ustrtoul(argv[2], 0, 10);
    ui32Cnt   = ustrtoul(argv[3], 0, 10);

    //
    // Update device address with the page that the user provided
    //
    memset(ui32Result,0x00,256);
    ui32UsrAddr = ADDRESS_24LC08B + ui32Page;
    eepromRead(ui32UsrAddr, ui32Start, ui32Result, ui32Cnt);

    for (i=0;i<ui32Cnt;i++){
        UARTprintf("%02x",ui32Result[i]);
    }
    UARTprintf("\n");

    // Return int
    return(0);

}


//*****************************************************************************
//
// Command: Set default data on external EEPROM
//
// Takes one arguments.
//     string - the string must be "force". Used to force the data reset / overwrite.
//
//*****************************************************************************
int CMD_setdata(int argc, char **argv)
{

    bool formatted = false;
    uint32_t ui32Result[8];
    uint32_t ui32Page;
    uint16_t ui16Start;
    uint32_t ui32Cnt;
    uint32_t ui32UsrAddr;

    if (argc > 2){
        UARTprintf("setdata Command Error. Type help for assistance.\n");
        return(1);
    }

    //
    // Test to see if the first byte of Page 2 is not 0xff.
    // This would indicate that the EEPROM has been set already.
    //

    memset(ui32Result,0x00,8);
    ui32Page = 2;
    ui16Start = 0;
    ui32Cnt = 8;   // FIXME: Grab 8 bytes for now. Grabbing one seems to produce an error.
    ui32UsrAddr = ADDRESS_24LC08B + ui32Page;
    eepromRead(ui32UsrAddr, ui16Start, ui32Result, ui32Cnt);
    if (ui32Result[0] != 0xff){
        formatted = true;
    }

    // Test to see if the EEPROM already has data.
    // If so, then require the user force the write.
    if (formatted){

        if (argc < 2){
            UARTprintf("setdata EEPROM has been formatted. Use force keyword to reset. Type help for assistance.\n");
            return(1);
        }

        if (!strncmp(argv[1], "force", 6)){
            UARTprintf("setdata forcing data reset.\n");
            formatted = false;
        } else {
            UARTprintf("setdata Command Error: %s. Type help for assistance.\n",argv[1]);
            return(1);
        }
    }

    if (!formatted){
        setEEPROMData();
    }

    // Return int
    return(0);
}

//*****************************************************************************
//
// Command: Clear all data on external EEPROM
//
// Takes one argument.
//     char password: user to provide a password
//
//*****************************************************************************
int CMD_clrdata(int argc, char **argv)
{
    int i;
    uint32_t ui32Page = 0;
    uint32_t ui32UsrAddr;
    uint8_t clr[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    //uint8_t clr[] = {0xab, 0xab, 0xab, 0xab, 0xab, 0xab, 0xab, 0xab, 0xab, 0xab, 0xab, 0xab, 0xab, 0xab, 0xab, 0xab};
    //uint8_t clr[] = {0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd};

    if (!(testPasswd(argv[1]))){
        UARTprintf("Incorrect password. This will be reported.\n");
        return(1);
    }

    for (ui32Page = 0; ui32Page < 4; ui32Page++){

        ui32UsrAddr = ADDRESS_24LC08B + ui32Page;
        for (i=0;i < 16;i++){
            eepromWrite(ui32UsrAddr, i * 16, clr, 16);
            // This delay seems to work best for writing
            // https://gist.github.com/ctring/7f12d812fb594eecc493
            ROM_SysCtlDelay( (ROM_SysCtlClockGet() / 3) / 100 );
        }
    }

    // Return int
    return(0);
}

//*****************************************************************************
//
// Command: Clear one page of data on external EEPROM
//
// Takes two arguments.
//     char password: user to provide a password
//     int page: number of the page (0-3) to erase
//
//*****************************************************************************
int CMD_clrpage(int argc, char **argv)
{
    int i;

    uint32_t ui32Page;
    uint32_t ui32UsrAddr;
    uint8_t clr[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    //uint8_t clr[] = {0xab, 0xab, 0xab, 0xab, 0xab, 0xab, 0xab, 0xab, 0xab, 0xab, 0xab, 0xab, 0xab, 0xab, 0xab, 0xab};
    //uint8_t clr[] = {0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd};

    //
    // This command requires one parameter.
    //
    if(argc != 3){
        UARTprintf("clrpage Command Error. Type help for assistance.\n");
        return(1);
    }

    if (!(testPasswd(argv[1]))){
        UARTprintf("Incorrect password. This will be reported.\n");
        return(1);
    }

    //
    // Extract the page number from the command line parameter.
    //
    ui32Page  = ustrtoul(argv[2], 0, 10);
    ui32UsrAddr = ADDRESS_24LC08B + ui32Page;

    //
    // Page must be a value between 0 and 3.
    //
    if(ui32Page > 3){
        UARTprintf("clrpage Command Error: page number. Type help for assistance.\n");
        return(1);
    }

    for (i=0;i < 16;i++){
        eepromWrite(ui32UsrAddr, i * 16, clr, 16);
        // This delay seems to work best for writing
        // https://gist.github.com/ctring/7f12d812fb594eecc493
        ROM_SysCtlDelay( (ROM_SysCtlClockGet() / 3) / 100 );
    }

    // Return int
    return(0);
}

//*****************************************************************************
//
// Command: Test that the user provided password is the password in EEPROM.
//
// Takes one argument.
//     char password: user to provide a password
//
//*****************************************************************************
int CMD_testpass(int argc, char **argv)
{
    //
    // This command requires one parameter.
    //
    if(argc != 2){
        UARTprintf("testpass Command Error. Type help for assistance.\n");
        return(1);
    }

    if (!(testPasswd(argv[1]))){
        UARTprintf("Incorrect password. This will be reported.\n");
        return(1);
    }

    // Return int
    return(0);
}
