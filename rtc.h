#ifndef __RTC_H
#define __RTC_H
#include "proj_consts.h"
#include "p24FJ64GA002.h"
//#include "./fatfs/src/integer.h"

// Union to access rtcc registers
typedef union tagRTCC {
	struct {
		unsigned char sec;
		unsigned char min;
		unsigned char hr;
		unsigned char wkd;
		unsigned char day;
		unsigned char mth;
		unsigned char yr;
	};
	struct {
		unsigned int prt00;
		unsigned int prt01;
		unsigned int prt10;
		unsigned int prt11;
	};
} RTCC;

#define mRTCCDec2Bin(Dec) (10*(Dec>>4)+(Dec&0x0f))
#define mRTCCBin2Dec(Bin) (((Bin/10)<<4)|(Bin%10))

typedef unsigned long	DWORD;
typedef unsigned short	WORD;

RTCC _time;
RTCC _time_chk;

void init_RTCC();
void RTCCGetDateTime(void);
void RTCCSet(void);
DWORD get_fattime(void);
void test_rtcc();
void RTCCCalculateWeekDay();
void RTCCUnlock();

#endif
