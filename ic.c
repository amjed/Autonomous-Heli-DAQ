//input capture
#include "p24FJ64GA002.h"
#include "ic.h"
#include "pps.h"
#include "proj_consts.h"

volatile unsigned char trig_edge ;
/***************	input cap 1	***************/
volatile unsigned int start_time_ic1;
volatile unsigned int pulse_time_ic1;

/***************	input cap 2	***************/
volatile unsigned int start_time_ic2;
volatile unsigned int pulse_time_ic2;

/***************	input cap 3	***************/
volatile unsigned int start_time_ic3;
volatile unsigned int pulse_time_ic3;

/***************	input cap 4	***************/
volatile unsigned int start_time_ic4;
volatile unsigned int pulse_time_ic4;
//volatile IC_SAMPLE ic_samples[IC_SAMPLE_SIZE];
volatile unsigned char ic_current_sample;

unsigned int get_pulse_time(unsigned char ch)
{
	switch(ch)
	{
		case 1:
			return pulse_time_ic1;
		case 2:
			return pulse_time_ic2;
		case 3:
			return pulse_time_ic3;
		case 4:
			return pulse_time_ic4;
		default:
			return 0;
	}

}
void init_input_capture()
{
	_IC1IF = _IC2IF = _IC3IF = _IC4IF = _IC5IF = 0;
	
	iPPSInput(IN_FN_PPS_IC1,IN_PIN_PPS_RP6);
	iPPSInput(IN_FN_PPS_IC2,IN_PIN_PPS_RP7);
	iPPSInput(IN_FN_PPS_IC3,IN_PIN_PPS_RP8);
	iPPSInput(IN_FN_PPS_IC4,IN_PIN_PPS_RP9);

	_TRISB6 = 1;
	_TRISB7 = 1;
	_TRISB8 = 1;
	_TRISB9 = 1;

	_T3IF = 0;
	PR3=0xFFFF;
	TMR3=0;
	T3CON = 0x8010;//1:8 2us resolution
	_T3IE = 0;
	
	IC1CON = 0x0003;//rising 0x0002 falling
	IC2CON = 0x0003; //change back to 0x0003
	IC3CON = 0x0003;
	IC4CON = 0x0003;

	trig_edge = 0b00001111;
	pulse_time_ic1 = pulse_time_ic2 = pulse_time_ic3 = pulse_time_ic4 = 0;
	start_time_ic1 = start_time_ic2=start_time_ic3=start_time_ic4=0;
	ic_current_sample = 0;
	_IC1IE = _IC2IE = _IC3IE = _IC4IE = 1;
}



void __attribute__((interrupt,no_auto_psv)) _IC1Interrupt(void)
{
	unsigned int temp;
	if(trig_edge & 1)
	{
		start_time_ic1 = IC1BUF;
		IC1CON=0x0000; //turn off IC1
		IC1CON=0x0002; //set to triger on falling edge
		trig_edge &= 0b11111110;
	}
	else
	{
		temp = IC1BUF;
		if(temp > start_time_ic1 )
			pulse_time_ic1 =  temp - start_time_ic1;
		else
			pulse_time_ic1 =  start_time_ic1 - temp ;
		IC1CON=0x0000; //turn off IC1
		IC1CON=0x0003;	//set to triger on rising edge
		trig_edge |= 0b00000001;
	}
    _IC1IF = 0;
}

void __attribute__((interrupt,no_auto_psv)) _IC2Interrupt(void)
{
	unsigned int temp;
	if(trig_edge & 2)
	{
		start_time_ic2 = IC2BUF;
		IC2CON=0x0000; //turn off IC1
		IC2CON=0x0002; //set to triger on falling edge
		trig_edge &= 0b11111101;
	}
	else
	{
		temp = IC2BUF;
		if(temp > start_time_ic2 )
			pulse_time_ic2 =  temp - start_time_ic2;
		else
			pulse_time_ic2 =  start_time_ic2 - temp ;
		IC2CON=0x0000; //turn off IC1
		IC2CON=0x0003;	//set to triger on rising edge
		trig_edge |= 0b00000010;
	}
    _IC2IF = 0;
}

void __attribute__((interrupt,no_auto_psv)) _IC3Interrupt(void)
{
	unsigned int temp;
	if(trig_edge & 4)
	{
		start_time_ic3 = IC3BUF;
		IC3CON=0x0000; //turn off IC1
		IC3CON=0x0002; //set to triger on falling edge
		trig_edge &= 0b11111011;
	}
	else
	{
		temp = IC3BUF;
		if(temp > start_time_ic3 )
			pulse_time_ic3 =  temp - start_time_ic3;
		else
			pulse_time_ic3 =  start_time_ic3 - temp ;
		IC3CON=0x0000; //turn off IC1
		IC3CON=0x0003;	//set to triger on rising edge
		trig_edge |= 0b00000100;
	}
    _IC3IF = 0;
}

void __attribute__((interrupt,no_auto_psv)) _IC4Interrupt(void)
{
	unsigned int temp;
	if(trig_edge & 8)
	{
		start_time_ic4 = IC4BUF;
		IC4CON=0x0000; //turn off IC1
		IC4CON=0x0002; //set to triger on falling edge
		trig_edge &= 0b11110111;
	}
	else
	{
		temp = IC4BUF;
		if(temp > start_time_ic4 )
			pulse_time_ic4 =  temp - start_time_ic4;
		else
			pulse_time_ic4 =  start_time_ic4 - temp ;
		IC4CON=0x0000; //turn off IC1
		IC4CON=0x0003;	//set to triger on rising edge
		trig_edge |= 0b00001000;
	}
    _IC4IF = 0;
}