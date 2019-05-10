/*
 * crypto.c
 *
 *  Created on: Apr 16, 2019
 *      Author: cutaway
 *
 *  Reference:
 *      kokke/tiny-AES-c - https://github.com/kokke/tiny-AES-c
 */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "aes.h"
#include "utils/uartstdio.h"
#include "crypto.h"

// Enable ECB, CTR and CBC mode. Note this can be done before including aes.h or at compile-time.
// E.g. with GCC by using the -D flag: gcc -c aes.c -DCBC=0 -DCTR=1 -DECB=1
#define CBC 1
#define CTR 1
#define ECB 1


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
bool g_bAESInitialized = false;
struct AES_ctx g_AESctx_ctx;

//
// Encryption key
//
//
// Something to store in firmware and eeprom. How about Base64 encoded encryption key: 16157e2ba6d2ae288815f7ab3c4fcf09
//
const char g_chEncodedKey[] = "MTYxNTdlMmJhNmQyYWUyODg4MTVmN2FiM2M0ZmNmMDk=";

char g_chAES128Key[] = "16157e2ba6d2ae288815f7ab3c4fcf09";
uint8_t g_ui8AES128Key[16] = {};

//
// Initial value for CBC mode.
//
char g_chAESIV[] = "00000000000000000000000000000000";
uint8_t g_ui8AESIV[16] = {};

//
// Default value for Plain Text
//
const char g_chAESPlainTextDefault[] = "e2bec16b969f402e117e3de92a179373578a2dae9cac031eac6fb79e518eaf45461cc83011e45ca319c1fbe5ef520a1a45249ff6179b4fdf7b412bad10376ce6";
uint8_t g_ui8AESPlainTextDefault[64] = {};

//
// The Cipher Text for default Plain Text:
//
const char g_chAESCipherTextDefault[] = "acab497646b219819b8ee9ce7d19e9129bcb8650ee1972503a11db95b2787691b8d6be733b74c1e39ee6167116952222a1caf13f09ac1f6830ca0e12a7e18675";
uint8_t g_ui8AESCipherTextDefault[64] = {};

//
// Storage for processed data
//
char g_chAESPlainText[MAX_STR];
uint8_t *g_ui8AESPlainText[MAX_STR/2];
char g_chAESCipherText[MAX_STR];
uint8_t *g_ui8AESCipherText[MAX_STR/2];


// Reference:
//    https://stackoverflow.com/questions/10324/convert-a-hexadecimal-string-to-an-integer-efficiently-in-c/11068850
//    https://stackoverflow.com/questions/10156409/convert-hex-string-char-to-int
// Modified from uint32_t to uint8_t
int hex2int(char *hex) {
    uint8_t val = 0;
    while (*hex) {
        // get current character then increment
        uint8_t byte = *hex++;
        // transform hex character to the 4bit equivalent number, using the ascii table indexes
        if (byte >= '0' && byte <= '9') byte = byte - '0';
        else if (byte >= 'a' && byte <='f') byte = byte - 'a' + 10;
        else if (byte >= 'A' && byte <='F') byte = byte - 'A' + 10;
        // shift 4 to make space for new digit, and add the 4 bits of the new digit
        val = (val << 4) | (byte & 0xF);
    }
    return val;
}

int set_value(char *in, uint8_t *out){
    int i, j, len;
    char *xhex[3];

    // Reference: https://www.linuxquestions.org/questions/programming-9/extract-substring-from-string-in-c-432620/
    len = strlen(in);
    for (i=0,j=0;i<len;i=i+2,j++){
        strncpy((char *) xhex,in+i,2);
        out[j] = hex2int((char *) xhex);
    }

    return(0);

}

int print_hex_value(uint8_t *in, int cnt){
    int i;

    // Reference: https://www.linuxquestions.org/questions/programming-9/extract-substring-from-string-in-c-432620/
    for (i=0;i<cnt;i++){
        //printf("%02x",in[i]);/
        UARTprintf("%02x",in[i]);
    }
    //printf("\n");
    UARTprintf("\n");
    return(0);
}

void rtn_hex_value(char* result, uint8_t *in, int cnt){
    int i;
    int len = 0;
    //int size = (BLOCK*2) + 1;

    // Reference: https://www.linuxquestions.org/questions/programming-9/extract-substring-from-string-in-c-432620/
    for (i=0;i<cnt;i++){
        len += snprintf(result+len,3,"%02x",in[i]);
    }
}

int print_hex_value_pkcs7(uint8_t *in, int cnt){
    int i, tcnt;

    if (in[cnt-1] < BLOCK){
        if (in[cnt-1] == 1){
            tcnt = cnt - 2;
        } else if (in[cnt-1] == in[cnt-2]){
            tcnt = cnt - in[cnt-1];
        } else {
            tcnt = cnt;
        }
    } else {
        tcnt = cnt;
    }

    // Reference: https://www.linuxquestions.org/questions/programming-9/extract-substring-from-string-in-c-432620/
    for (i=0;i<tcnt;i++){
        //printf("%02x",in[i]);/
        UARTprintf("%02x",in[i]);
    }
    //printf("\n");
    UARTprintf("\n");
    return(0);
}


void rtn_hex_value_pkcs7(char* result, uint8_t *in, int cnt){
    int i, tcnt;
    int len = 0;

    if (in[cnt-1] < BLOCK){
        if (in[cnt-1] == 0){
            tcnt = cnt - 1;
        } else if (in[cnt-1] == 1){
            tcnt = cnt - 2;
        } else if (in[cnt-1] == in[cnt-2]){
            tcnt = cnt - in[cnt-1];
        } else {
            tcnt = cnt;
        }
    } else {
        tcnt = cnt;
    }

    // Reference: https://www.linuxquestions.org/questions/programming-9/extract-substring-from-string-in-c-432620/
    for (i=0;i<tcnt;i++){
        len += snprintf(result+len,3,"%02x",in[i]);
    }
}

void dec_blocks(char* output, uint8_t *in, size_t len){

    uint8_t tmp[BLOCK];

    int cnt, cycle = 0;
    int passes = (len / BLOCK);
    int out_size = (len*2) + 1;

    char *rtn;
    rtn = (char*) malloc(out_size * sizeof(char));
    if (rtn == NULL){
        UARTprintf("Malloc Failed.\n");
        return;
    }
    memset(rtn,0x00,out_size);

    // Run until our cycles equals the number of passes
    while (passes > cycle){
        // Clear out tmp, just in case
        // NOTE: I don't think this actually works.
        memset((char*) tmp,0x0,BLOCK);

        // Build the block to be encrypted byte-by-byte
        for (cnt = 0; cnt < BLOCK; cnt++){
            // Detect last pass. Add one to cycle since we increment at end of while loop.
            // NOTE: Is there a better way to test this?
            tmp[cnt] = in[cnt + (BLOCK*cycle)];
        }

        // Decryption
        AES_init_ctx(&g_AESctx_ctx, g_ui8AES128Key);
        AES_ECB_decrypt(&g_AESctx_ctx, tmp);
        // Test each uint8_t value
        // NOTE: This should return the value. Although it might
        // NOTE: be in the same place that it was passed in in memory.
        rtn_hex_value_pkcs7(rtn,tmp,BLOCK);
        /* FIXME: There is a one-off issue here. This is an example. CyberChef indicates
         *        that this is the correct encryption. Thus the string print / concantenation
         *        is the issue.
         *
         *   > encbytes deadbeefdeadbeefdeadbeefdeadbeefdeadbeefdeadbeefdeadbeefdeadbe
         *   a121f3f058201bc325ffbe20d132e7a5eed7ee1639842d46633d1259b07def05
         *
         *
         *   > decbytes a121f3f058201bc325ffbe20d132e7a5eed7ee1639842d46633d1259b07def05
         *   deadbeefdeadbeefdeadbeefdeadbeefdeadbeefdeadbeefdeadbeefdead
         */
        strncat(output,rtn,BLOCK*2);
        cycle++;
    }
    free(rtn);
}

void enc_blocks(char* output, uint8_t *in, size_t len){

    uint8_t tmp[BLOCK];

    int cnt, cycle = 0;
    int passes, out_size = 1;
    int pad = BLOCK - (len % BLOCK);
    if (pad == BLOCK){
        pad = 0;
    }

    if (len % BLOCK){
        passes = (len / BLOCK) + 1;
    } else {
        passes = (len / BLOCK);
    }
    out_size = ((BLOCK * passes) * 2) + 1;

    char *rtn;
    rtn = (char*) malloc(out_size * sizeof(char));
    if (rtn == NULL){
        UARTprintf("Malloc Failed.\n");
        return;
    }
    memset(rtn,0x00,out_size);


    // Run until our cycles equals the number of passes
    while (passes != cycle){
        // Clear out tmp, just in case
        // NOTE: I don't think this actually works.
        memset((char*) tmp,0x0,BLOCK);

        // Build the block to be encrypted byte-by-byte
        for (cnt = 0; cnt < BLOCK; cnt++){
            // Detect last pass. Add one to cycle since we increment at end of while loop.
            // NOTE: Is there a better way to test this?
            if (passes - (cycle + 1)){
                tmp[cnt] = in[cnt + (BLOCK*cycle)];
            } else {
                // Put the final bytes into block, then pad using PKCS#7
                if (cnt < (BLOCK - pad)){
                    tmp[cnt] = in[cnt + (BLOCK*cycle)];
                } else {
                    tmp[cnt] = pad;
                }
            }
        }

        // Encryption
        AES_init_ctx(&g_AESctx_ctx, g_ui8AES128Key);
        AES_ECB_encrypt(&g_AESctx_ctx, tmp);

        // Test each uint8_t value
        // NOTE: This should return the value. Although it might
        // NOTE: be in the same place that it was passed in in memory.
        rtn_hex_value(rtn,tmp,BLOCK);
        strncat(output,rtn,BLOCK*2);
        cycle++;
    }
    free(rtn);
}


//*****************************************************************************
//
// Command: Initialize AES Encryption
//
// Set up device for AES Encryption / Decryption
//
//*****************************************************************************
int SET_aes()
{
    g_bAESInitialized = true;
    // Set key and IV
    set_value(&g_chAES128Key, &g_ui8AES128Key);

    /* See https://github.com/kokke/tiny-AES-c/blob/master/test.c */
    /* Initialize context calling one of: */
    //void AES_init_ctx(struct AES_ctx* ctx, const uint8_t* key);
    void AES_init_ctx(struct AES_ctx* g_AESctx_ctx, const uint8_t* g_ui8AES128Key);
    //void AES_init_ctx_iv(struct AES_ctx* ctx, const uint8_t* key, const uint8_t* iv);
    void AES_init_ctx_iv(struct AES_ctx* g_AESctx_ctx, const uint8_t* g_ui8AES128Key, const uint8_t* g_ui8AESIV);

    /* ... or reset IV at random point: */
    void AES_ctx_set_iv(struct AES_ctx* g_AESctx_ctx, const uint8_t* g_ui8AESIV);

    // Return int
    return(0);

}

//*****************************************************************************
//
// Command: Check if AES Encryption has been initialized
//
// Determine if device is set up for AES Encryption / Decryption
//
//*****************************************************************************
bool GET_aes()
{
    // Return int
    return(g_bAESInitialized);

}
//*****************************************************************************
//
// Command: Check if AES Encryption has been initialized
//
// Determine if device is set up for AES Encryption / Decryption
//
//*****************************************************************************
int RESET_aes()
{
    // Return int
    g_bAESInitialized = false;
    return(0);

}
