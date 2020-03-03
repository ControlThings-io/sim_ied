/*
 * interface.h
 *
 *  Created on: Apr 15, 2019
 *      Author: cutaway
 */

#ifndef INTERFACE_H_
#define INTERFACE_H_

//*****************************************************************************
//
// Globally defined constants
//
//*****************************************************************************
#define APP_INPUT_BUF_SIZE               128
//#define USE_EEPROM

//*****************************************************************************
//
// Globally variables
//
//*****************************************************************************
extern const char g_chVersion[];
extern const char g_chName[];
extern const char g_chVendor[];
extern const char g_chAuthor[];
extern const char g_chContact[];
extern const char g_chWebsite[];


//*****************************************************************************
//
// Functions defined in interface.c that are made available to other files.
//
//*****************************************************************************
//extern void AppButtonHandler(void);
//extern void AppRainbow(uint32_t ui32ForceUpdate);
//extern void AppHibernateEnter(void);



#endif /* INTERFACE_H_ */
