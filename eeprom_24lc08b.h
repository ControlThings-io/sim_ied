/*
 * eeprom_24lc08b.h
 *
 *  Created on: Apr 30, 2019
 *      Author: cutaway
 */


#include <stdint.h>

#ifndef EEPROM_24LC08B_H_
#define EEPROM_24LC08B_H_


// 24LC08B slave address is 0xA0 by default but it is 0x50 is the 7-bit representation
#define ADDRESS_24LC08B 0x50

//
// Something to store. How about Base64 encoded encryption key: 16157e2ba6d2ae288815f7ab3c4fcf09
//
//extern const char g_chEncodedKey[];

//extern void ConfigureI2C1();
extern void ROM_ConfigureI2C1();
extern void eepromRead(uint32_t, uint16_t, uint32_t *, uint32_t );
extern void eepromWrite(uint32_t, uint16_t, uint8_t *, uint32_t);
extern void resetPtrEEPROM();

#endif /* EEPROM_24LC08B_H_ */
