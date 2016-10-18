//ultrasonic
#include "p24FJ64GA002.h"
#include "us.h"
#include "PPS.h"
#include "proj_consts.h"

volatile unsigned char dist[4];
volatile unsigned char ptr_dist=0;
volatile unsigned char flags_us = 0;

volatile US_SAMPLE US_samples[US_SAMPLE_SIZE];
volatile US_SAMPLE US_samples_2[US_SAMPLE_SIZE];

volatile unsigned char us_current_sample=0;
volatile unsigned char us_active_buffer=0;

unsigned char us_get_sample_count()
{
	return (unsigned char)US_SAMPLE_SIZE;
}

US_SAMPLE*	us_get_sample_buffer()
{
	if(!us_active_buffer)
		return &US_samples;
	else
		return &US_samples_2;
}
void init_ultrasonic()
{
	
	//configure UART2 for ultrasonic sensor
	U2BRG=(FCY/BAUD/16)-1;
	U2MODEbits.UEN = 0;
	U2MODEbits.BRGH = 0;
	U2STA= 0x0400;
	U2MODEbits.UARTEN = 1;
	IFS1bits.U2RXIF=0;
	IPC7bits.U2RXIP = 7;
	IEC1bits.U2RXIE = 1;
	//assign i to uart2
	TRISBbits.TRISB2 = 1;
	iPPSInput(IN_FN_PPS_U2RX,IN_PIN_PPS_RP2);
}

void __attribute__ ((interrupt,no_auto_psv)) _U2RXInterrupt(void)
{
	unsigned char temp;
	IFS1bits.U2RXIF=0;
	temp = U2RXREG;
	if(U1STAbits.OERR)
	{
		U1STAbits.OERR = 0;
	}
	if(temp == 'R' && !(flags_us&1))
	{
		flags_us |= 1;
	}
	else if(flags_us&1)
	{
		dist[ptr_dist++] = temp;
		if(ptr_dist == 4)
		{
			flags_us = 0;
			flags_us |=0X80;
			ptr_dist=0;		
			if(!us_active_buffer)	
			{
 				US_samples[us_current_sample].dist[0] = dist[0];
				US_samples[us_current_sample].dist[1] = dist[1];
				US_samples[us_current_sample].dist[2] = dist[2];
				US_samples[us_current_sample].dist[3] = dist[3];
				US_samples[us_current_sample].time = get_time_word();
				us_current_sample++;
				if(us_current_sample >= US_SAMPLE_SIZE)
				{
					us_current_sample = 0;
					us_active_buffer ^=1;
					time_to_write_us = 1;
				}
				
			}
			else
			{
			 	US_samples_2[us_current_sample].dist[0] = dist[0];
				US_samples_2[us_current_sample].dist[1] = dist[1];
				US_samples_2[us_current_sample].dist[2] = dist[2];
				US_samples_2[us_current_sample].dist[3] = dist[3];
				US_samples_2[us_current_sample].time = get_time_word();
				us_current_sample++;
				if(us_current_sample >= US_SAMPLE_SIZE)
				{
					us_current_sample = 0;
					us_active_buffer ^=1;
					time_to_write_us = 1;
					
				}
			}

		}	
		

	}
}	