/*
 * commands.h
 *
 *  Created on: Apr 21, 2019
 *      Author: cutaway
 */

#ifndef COMMANDS_H_
#define COMMANDS_H_

extern int CMD_help(int, char **);
extern int CMD_encbytes(int, char **);
extern int CMD_decbytes(int, char **);
extern int CMD_gkey(int, char **);
extern int CMD_skey(int, char **);
extern int CMD_dumpdata(int, char **);
extern int CMD_getdata(int, char **);
extern int CMD_setdata(int, char **);
extern int CMD_clrdata(int, char **);
extern int CMD_clrpage(int, char **);
//extern int CMD_testpass(int, char **);

// Helper functions
extern int writeString(uint32_t, uint16_t, char *, uint32_t);
extern void setEEPROMData();
extern bool testPasswd(char *);

#endif /* COMMANDS_H_ */
