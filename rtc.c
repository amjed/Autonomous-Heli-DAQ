//rtc
#include "rtc.h"

/*********************************************************************
 * Function: RTCCProcessEvents
 *
 * Preconditions: RTCCInit must be called before.
 *
 * Overview: The function grabs the current time from the RTCC and
 * translate it into strings.
 *
 * Input: None.
 *
 * Output: It update time and date strings  _time_str, _date_str,
 * and _time, _time_chk structures.
 *
 ********************************************************************/
void RTCCGetDateTime(void)
{
	// Process time object only if time is not being set

		// Grab the time
		RCFGCALbits.RTCPTR = 3;			
		_time.prt11 = RTCVAL;
		_time.prt10 = RTCVAL;
		_time.prt01 = RTCVAL;
		_time.prt00 = RTCVAL;

		// Grab the time again 
		RCFGCALbits.RTCPTR = 3;			
		_time_chk.prt11 = RTCVAL;
		_time_chk.prt10 = RTCVAL;
		_time_chk.prt01 = RTCVAL;
		_time_chk.prt00 = RTCVAL;

		// Verify there is no roll-over
		if ((_time.prt00 == _time_chk.prt00) &&
			(_time.prt01 == _time_chk.prt01) &&
			(_time.prt10 == _time_chk.prt10) &&
			(_time.prt11 == _time_chk.prt11))
		{
			/*_time.yr = _time.prt11;
			
			_time.mth = _time.prt10>>8;
			_time.day = _time.prt10;
			
			_time.wkd = _time.prt01>>8;
			_time.hr = _time.prt01;
			
			_time.min = _time.prt01>>8;
			_time.sec = _time.prt01;*/
			 _time.sec = mRTCCDec2Bin(_time.sec);
			 _time.min = mRTCCDec2Bin(_time.min);
			 _time.hr = mRTCCDec2Bin(_time.hr);
			 _time.day = mRTCCDec2Bin(_time.day);
			    _time.mth = mRTCCDec2Bin(_time.mth);
			    _time.yr = mRTCCDec2Bin(_time.yr);
			
		}
	
}

/*********************************************************************
 * Function: RTCCInit
 *
 * Preconditions: RTCCInit must be called before.
 *
 * Overview: Enable the oscillator for the RTCC
 *
 * Input: None.
 *
 * Output: None.
 ********************************************************************/
void init_RTCC(void)
{
    // Enables the LP OSC for RTCC operation
	 __builtin_write_OSCCONL(OSCCON | 0x0002);
	asm("mov.b #0x02, W0");	// move 8-bit literal to W0, 16-bit.
	asm("mov.b #0x46, W2");	// unlock byte 1 for OSCCONL(low byte)
	asm("mov.b #0x57, W3");	// unlock byte 2 for OSCCONL(low byte)
							// move 8-bit of Wn to OSCCON register
	asm("mov.b W2, [W1]");	// write unlock byte 1
	asm("mov.b W3, [W1]");	// write unlock byte 2
	asm("mov.b W0, [W1]");	// enable SOSCEN

    // Unlock sequence must take place for RTCEN to be written
	RCFGCAL	= 0x0000;			
    RTCCUnlock();
    RCFGCALbits.RTCEN = 1;	// bit15
    
    //RTCC pin pad conected to RTCC second clock
	PADCFG1bits.RTSECSEL = 1;	
	RCFGCALbits.RTCOE = 0;		//RTCC Output Enable bit

	/* Enable the RTCC interrupt*/
	IFS3bits.RTCIF = 0;		//clear the RTCC interrupt flag
	IEC3bits.RTCIE = 0;		//enable the RTCC interrupt

    // TO DO: Write the time and date to RTCC as follow. 
	_time_chk.sec = 0x00;
	_time_chk.min = 0;
	_time_chk.hr = 0x0;
	_time_chk.wkd = 0x0; 
	_time_chk.day = 0x0;
	_time_chk.mth = 0x00;
	_time_chk.yr = 0x09;
	RTCCCalculateWeekDay();	// To calculate and confirm the weekday

	// Set it after you change the time and date. 
	RTCCSet();
/*	
	// Set alarm
	_alarm.sec	= 0x01;
	_alarm.min	= 0x01;
	_alarm.hr	= 0x01;
	_alarm.wkd	= 0x01;
	_alarm.day	= 0x01;
	_alarm.mth	= 0x01;
	RTCCALMSet();*/
}

/*********************************************************************
 * Function: RTCCSet
 *
 * Preconditions: None.
 *
 * Overview: 
 * The function upload time and date from _time_chk into clock.
 *
 * Input: _time_chk - structure containing time and date.
 *
 * Output: None.
 *
 ********************************************************************/
void RTCCSet(void)
{
	RTCCUnlock();				// Unlock the RTCC
	
	RCFGCALbits.RTCPTR = 3;		// Set the time
	RTCVAL = _time_chk.prt11;	// set year
	RTCVAL = _time_chk.prt10;	// set month:day
	RTCVAL = _time_chk.prt01;	// set week:hour
	RTCVAL = _time_chk.prt00;	// set min:sec

	RCFGCALbits.RTCWREN = 0;	// Lock the RTCC
	
	// Here, you can watch the RTCC register, 
	// 	the new time and date has been updated. 
}
void RTCCUnlock()
{/*
	asm volatile("disi	#5");
	asm volatile("mov #0x55, w7");		// write 0x55 and 0xAA to
	asm volatile("mov w7, _NVMKEY"); 	//  NVMKEY to disable
	asm volatile("mov #0xAA, w8");		// 	write protection
	asm volatile("mov w8, _NVMKEY");
    asm volatile("bset _RCFGCAL, #13");	// set the RTCWREN bit
	asm volatile("nop");
	asm volatile("nop");*/
	__builtin_write_RTCWEN(); 
}

void RTCCCalculateWeekDay()
{
	const char MonthOffset[] =
	//jan feb mar apr may jun jul aug sep oct nov dec
	{   0,  3,  3,  6,  1,  4,  6,  2,  5,  0,  3,  5 };
	unsigned Year;
	unsigned Month;
	unsigned Day;
	unsigned Offset;
    // calculate week day 
    Year  = mRTCCDec2Bin(_time_chk.yr);
    Month = mRTCCDec2Bin(_time_chk.mth);
    Day  = mRTCCDec2Bin(_time_chk.day);
    
    // 2000s century offset = 6 +
    // every year 365%7 = 1 day shift +
    // every leap year adds 1 day
    Offset = 6 + Year + Year/4;
    // Add month offset from table
    Offset += MonthOffset[Month-1];
    // Add day
    Offset += Day;

    // If it's a leap year and before March there's no additional day yet
    if((Year%4) == 0)
        if(Month < 3)
            Offset -= 1;
    
    // Week day is
    Offset %= 7;

    _time_chk.wkd = Offset;
}
DWORD get_fattime (void)
{
	DWORD tmr;

	RTCCGetDateTime();
	

	// Pack date and time into a DWORD variable 
	tmr =	  (((DWORD)_time.yr ) << 25)
			| ((DWORD)_time.mth << 21)
			| ((DWORD)_time.wkd << 16)
			| (WORD)(_time.hr << 11)
			| (WORD)(_time.min << 5)
			| (WORD)(_time.sec >> 1);


	return tmr;
}

void test_rtcc()
{
	RTCCGetDateTime();
	printf("time: %d:%d:%d %d/%d/%d\r\n",_time.hr,
										_time.min,
										_time.sec,
										_time.wkd,
										_time.mth,
										_time.yr);
}										