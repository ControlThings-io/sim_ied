/*
 * crypto.h
 *
 *  Created on: Apr 16, 2019
 *      Author: cutaway
 */


#include <stdint.h>

#ifndef CRYPTO_H_
#define CRYPTO_H_

// Only supporting AES128 at this time.
#ifndef AES128
#define AES128
#endif
//#define AES192
//#define AES256

#define BLOCK   16
#define MAX_KEY 16
#define MAX_IV  16
#define MAX_STR 256

// Global Variables


//*****************************************************************************
//
// Default encryption information. Key, IV, Plain Text
//
// AES Example from TivaWare Peripheral Driver Library User's Guide
// Located in Code Composer Studio under
// Software -> TM4C ARM -> Libraries -> Driver Library -> User's Guide
//
//*****************************************************************************
//
// Encryption Initialization Tracker
//
//extern bool g_bAESInitialized;
//extern struct AES_ctx g_AESctx_ctx;

//
// Encryption key
//
extern const char g_chEncodedKey[];
extern char g_chAES128Key[];
extern uint8_t g_ui8AES128Key[];

//
// Initial value for CBC mode.
//
extern char g_chAESIV[];
extern uint8_t g_ui8AESIV[];

//
// Default value for Plain Text
//
extern const char g_chAESPlainTextDefault[];
extern uint8_t g_ui8AESPlainTextDefault[];

//
// The Cipher Text for default Plain Text:
//
extern const char g_chAESCipherTextDefault[];
extern uint8_t g_ui8AESCipherTextDefault[];

//
// Storage for processed data
//

extern char g_chAESPlainText[MAX_STR];
extern uint8_t *g_ui8AESPlainText[MAX_STR/2];
extern char g_chAESCipherText[MAX_STR];
extern uint8_t *g_ui8AESCipherText[MAX_STR/2];

// Global Functions
extern int hex2int(char *);
extern int set_value(char *, uint8_t *);
extern int print_hex_value(uint8_t *, int);
extern void rtn_hex_value(char *, uint8_t *, int);
extern int print_hex_value_pkcs7(uint8_t *, int);
extern void rtn_hex_value_pkcs7(char *, uint8_t *, int);
extern void enc_blocks(char *, uint8_t *, size_t);
extern void dec_blocks(char*, uint8_t *, size_t);
extern int SET_aes();
extern bool GET_aes();
extern int RESET_aes();

#endif /* CRYPTO_H_ */
