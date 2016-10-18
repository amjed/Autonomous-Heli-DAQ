#include "p24FJ64GA002.h"
#include "uart.h"
#include "gps.h"
#include "PPS.h"
#include "proj_consts.h"

volatile GPS_SAMPLE GPS_samples[GPS_SAMPLE_SIZE];
volatile GPS_SAMPLE GPS_samples_2[GPS_SAMPLE_SIZE];

volatile unsigned char current_sample=0;
volatile unsigned char gps_active_buffer=0;

volatile unsigned char heading[6];
volatile unsigned char velocity[6];
volatile unsigned char longitude[10];
volatile unsigned char latitude[9];

volatile unsigned char buffer[50];
volatile unsigned char ptr=0,temp;
volatile unsigned char flags = 0;


/*****************************************************************
flags bit description :											  
	bit 7 | bit 6 | bit 5 | bit 4 | bit 3 | bit 2 | bit 1 | bit 0 | 
	 lock | head  |   0   |    0  |   0   |   0   |   0   |  sent 

lock: got gps lock and has at least one sample of position
head: got heading data
sent: currently receiving a sentance
0s  : unimplemented
*****************************************************************/


GPS_SAMPLE*	gps_get_sample_buffer()
{
	if(!gps_active_buffer)
		return &GPS_samples;
	else
		return &GPS_samples_2;
}
unsigned char gps_get_sample_count()
{
	return (unsigned char)GPS_SAMPLE_SIZE;
}
void init_gps()
{
	
	//configure UART1 for GPS
	U1BRG=(FCY/(16*BAUD))-1;
	U1MODEbits.UEN = 0;
	U1MODEbits.BRGH = 0;
	U1STA= 0x0400;
	U1MODEbits.UARTEN = 1;
	IFS0bits.U1RXIF=0;
	IPC2bits.U1RXIP = 7;
	IEC0bits.U1RXIE = 1;
	//assign i/o to uart
	//Assing U1TX to pin RP0,U1RX to pin RP1
	TRISBbits.TRISB14 = 1;
	iPPSOutput(OUT_PIN_PPS_RP13,OUT_FN_PPS_U1TX);
	iPPSInput(IN_FN_PPS_U1RX,IN_PIN_PPS_RP14);
   
	flags = 0;
	current_sample=0;
}

void gps_get_flag()
{
	return flags;
}
void __attribute__ ((interrupt,no_auto_psv)) _U1RXInterrupt(void)
{
	unsigned char temp,i;
	IFS0bits.U1RXIF=0;
	if(U1STAbits.OERR)
	{
		U1STAbits.OERR = 0;
	}	
	temp = U1RXREG;
	//first bit of flag signs the reception of '$' i.e new sentance
	//if current received char is '$' and there isnt a sentence in progress(flag&0x1 = 0) then set
	if(temp == '$' && !(flags&1))
	{
		ptr =0;
		flags |= 1;
	}	
	else//else if current rx char isnt '$'
	{
		if(temp == '$')//if char is '$' flag LSB must've been set i.e we have one sentence in buffer
		{
			flags &= 0b11111111;//reset flag
			ptr = 0;
			if(buffer[2] == 'G' && buffer[3] == 'L' &&  buffer[4] == 'L')//if sentence in buffer is GLL
			{
				for(i=0;i<strlen(buffer);i++)//decode and store appopriatly
				{
				
					if(buffer[i] == 'A')//got satellite lock ?
					{
						i=6;//skip to latitude data
						do
						{
							if(!gps_active_buffer)
								GPS_samples[current_sample].latitude[i-6] = buffer[i];//copy latitude
							else
								GPS_samples_2[current_sample].latitude[i-6] = buffer[i];//copy latitude
							i++;
						}
						while(buffer[i] != ',' && ((i-6)<10));
							
						if(!gps_active_buffer)
						{
							GPS_samples[current_sample].latitude[i-6] = buffer[++i];//copy direction indicator N or S
							GPS_samples[current_sample].latitude[i-6] = '\0';						
						}
						else
						{
							GPS_samples_2[current_sample].latitude[i-6] = buffer[++i];//copy direction indicator N or S
							GPS_samples_2[current_sample].latitude[i-6] = '\0';						
						}
						i+=2;//skip ',' to longitude data
						
						ptr = 0;
						do
						{
							if(!gps_active_buffer)
								GPS_samples[current_sample].longitude[ptr] = buffer[i];//copy longitude
							else
								GPS_samples_2[current_sample].longitude[ptr] = buffer[i];//copy longitude
							i++;ptr++;
						}
						while(buffer[i] != ',' && ptr<9);
						
						if(!gps_active_buffer)
						{
							GPS_samples[current_sample].longitude[ptr++] = buffer[++i];
							GPS_samples[current_sample].longitude[ptr++] = '\0';
						}
						else
						{
							GPS_samples_2[current_sample].longitude[ptr++] = buffer[++i];
							GPS_samples_2[current_sample].longitude[ptr++] = '\0';
						}						

						flags |=0x80;//toggle MSB to signal we have recent good position
						break;
					}
				}	
						
			
			
			}
			//if sentence is VTG and we got signal lock
			if(buffer[2] == 'V' && buffer[3] == 'T' &&  buffer[4] == 'G' && flags & 0x80)
			{
				i = 0;
				while(buffer[i] != ',')
					i++;
				temp = 0;i++;
				do
				{
					if(!gps_active_buffer)
						GPS_samples[current_sample].heading[temp++] = buffer[i++];
					else
						GPS_samples_2[current_sample].heading[temp++] = buffer[i++];
				}
				while(buffer[i] != ',');
				
				temp = strlen(buffer);//get buffer length
				ptr =0;//reset ptr (holds how many ',' encountered)
				
				for(i=0;i<temp;i++)
				{
					if(buffer[i] == ',')//if comma found
					{
						ptr++;
						if(ptr == 7)//if the 7th comma found
						{
							ptr = i;//store its index in ptr and quit searching
							break;
						}	
					}		
				}	
				if(++i < temp)//if the index is good
				{
					temp = 0;
					do
					{
						if(!gps_active_buffer)
							GPS_samples[current_sample].velocity[temp++] = buffer[i++];//copy velocity from buffer
						else
							GPS_samples_2[current_sample].velocity[temp++] = buffer[i++];//copy velocity from buffer
					}while(buffer[i] != ',' && temp<6);
					
					if(!gps_active_buffer)
					{
						GPS_samples[current_sample].velocity[5] = '\0';
						GPS_samples[current_sample].time = get_time_word();
					}
					else
					{
						GPS_samples_2[current_sample].velocity[5] = '\0';
						GPS_samples_2[current_sample].time = get_time_word();
					}
					
					flags |=0x40;//signal MSB-1 to indicate we got heading data
					current_sample++;
					if(current_sample > 2)
					{
						current_sample = 0;
						gps_active_buffer ^=1;
						time_to_write_gps = 1;
					}	
					
				}
			
			}
		}	
		else if(flags&1)
		{
			if(ptr<49)
				buffer[ptr++] = temp;
			else
			{
				ptr =0;
				flags &= 0b11111110;
			}	
		}
			
				
	}
		
}
