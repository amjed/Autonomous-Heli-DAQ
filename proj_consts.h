#ifndef __proj_consts_H
#define __proj_constsS_H


#define FCY 16000000UL

#define BAUD 19200UL
#define USE_AND_OR

#define  _DI __asm__ volatile("disi #0x3FFF");
#define  _EI __asm__ volatile("disi #0x0000");

void get_time(unsigned char *ptr);
unsigned long get_time_word();


#define L_LED	_LATB12
#define R_LED	_LATB11

#define __DEBUGGING 						1	//uses GPS port as general purpose UART

#endif