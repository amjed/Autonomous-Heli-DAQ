#define __GENERIC_TYPE_DEFS_H_ 1
#include "proj_consts.h"
#include "p24FJ64GA002.h"
//_CONFIG2(FNOSC_FRC & FCKSM_CSDCMD)
_CONFIG2( FCKSM_CSDCMD & OSCIOFNC_OFF & POSCMOD_HS & FNOSC_PRIPLL)
_CONFIG1(JTAGEN_OFF & GWRP_OFF & BKBUG_OFF& COE_OFF & FWDTEN_OFF & WINDIS_OFF & ICS_PGx3)


#include <libpic30.h> 

#include "gps.h"
#include "diskio.h"
#include "fatfs/src/ff.h"
#include "pps.h"
#include "ic.h"
#include "us.h"
#include "imu.h"


volatile UINT Timer;		/* 1kHz increment timer */

volatile BYTE rtcHour, rtcMin, rtcSec;
static volatile unsigned char time_str[14];
static volatile unsigned long w_time;

volatile unsigned char time_to_write_imu = 0;
volatile unsigned char time_to_write_gps= 0;
volatile unsigned char time_to_write_us= 0;
volatile unsigned char time_to_write_ic= 0;

FIL	file;

#define _VN_SEL()	 _LATA1 = 0
#define _VN_DSEL()	 _LATA1 = 1	
#define xmit_spi(dat) 	xchg_spi(dat)
#define rcvr_spi()	xchg_spi(0xFF)

void get_time(unsigned char *ptr)
{
	strcpy(ptr,time_str);
}
unsigned long get_time_word()
{
	return w_time;

}
void put_rc (FRESULT rc)
{
	const char *p;
	static const char str[] =
		"OK\0" "DISK_ERR\0" "INT_ERR\0" "NOT_READY\0" "NO_FILE\0" "NO_PATH\0"
		"INVALID_NAME\0" "DENIED\0" "EXIST\0" "INVALID_OBJECT\0" "WRITE_PROTECTED\0"
		"INVALID_DRIVE\0" "NOT_ENABLED\0" "NO_FILE_SYSTEM\0" "MKFS_ABORTED\0" "TIMEOUT\0";
	FRESULT i;

	for (p = str, i = 0; i != rc && *p; i++) {
		while(*p++);
	}
	printf("rc=%u FR_%s\n", (UINT)rc, p);
}

BYTE xchg_spi (BYTE dat)
{
	unsigned char temp;
	temp =  SPI1BUF;
	SPI1BUF = dat;
	while (!_SPIRBF);
	return (BYTE)SPI1BUF;
}



void __attribute__((interrupt, auto_psv)) _T1Interrupt (void)
{
	static UINT div1k;

	_T1IF = 0;		/* Clear irq flag */
	Timer++;			/* Performance counter for this module */
	disk_timerproc();	/* Drive timer procedure of low level disk I/O module */

	/* Real Time Clock */
	if (++div1k >= 1000) {
		
		div1k = 0;
		if (++rtcSec >= 60) {
			rtcSec = 0;
			if (++rtcMin >= 60) {
				rtcMin = 0;
				if (++rtcHour >= 24) 
					rtcHour = 0;

				
			}
		}
	}
	w_time = rtcMin << 16;
	w_time |= rtcSec << 10;
	w_time |= div1k;
	sprintf(time_str,"%d:%d:%d:%d\0",rtcHour,rtcMin,rtcSec,div1k);//takes ~ 110us
}

void init_ext_int()
{
	TRISBbits.TRISB10 = 1;
	_CN16PUE = 1;
	
	_INT1EP = 1;
	_INT1IE = 1;
	_INT1IP = 6;
	iPPSInput(IN_FN_PPS_INT1,IN_PIN_PPS_RP10);
}

void init_timer()
{
	PR1 = FCY / 8 / 1000;
	_TCKPS0 = 1;	
	_TON = 1;	
	_T1IE = 1;	
	_T1IP = 7;
}

void init_spi_SD()
{
	
	iPPSOutput(OUT_PIN_PPS_RP3,OUT_FN_PPS_SDO1);
	iPPSOutput(OUT_PIN_PPS_RP4,OUT_FN_PPS_SCK1OUT);
	iPPSInput(IN_FN_PPS_SDI1,IN_PIN_PPS_RP5);
	
	TRISBbits.TRISB5 = 1; //sdi


	_CN7PUE = 1; //pull up sdo
	_CN27PUE = 1; // pull up sdo
	
	_SPIEN = 0;
	SPI1CON1 = 0b0000000100111011;
	_SPIEN = 1;
}

void init_spi_VN()
{
	iPPSOutput(OUT_PIN_PPS_RP0,OUT_FN_PPS_SDO2);
	iPPSOutput(OUT_PIN_PPS_RP1,OUT_FN_PPS_SCK2OUT);
	iPPSInput(IN_FN_PPS_SDI2,IN_PIN_PPS_RP2);
	
	TRISBbits.TRISB2 = 1; //sdi

	
	SPI2STATbits.SPIEN = 0;
	SPI2CON1 = 0b0000000001111011;
	SPI2STATbits.SPIEN = 1;	
}

void init_uart()
{
	iPPSOutput(OUT_PIN_PPS_RP13,OUT_FN_PPS_U1TX);
	iPPSInput(IN_FN_PPS_U1RX,IN_PIN_PPS_RP14);
	
	TRISBbits.TRISB13 = 0; //TX
	TRISBbits.TRISB14 = 1; //RX
	
	_U1RXIE = 0;
	_U1TXIE = 0;
	_UARTEN = 0;
	/* Initialize UART1 */
	U1BRG = FCY / 16 / BAUD - 1;
	_UARTEN = 1;
	_UTXEN = 1;
}
void IoInit ()
{
	/* Initialize GPIO ports */
	AD1PCFG = 0xFFFF;//disable all analog inputs
	LATB =  0x00;
	TRISB = 0x00;
	LATA =  0x0000;
	TRISA = 0x0000;
	
	init_timer();
	
	_LATA0 = 1;
	_LATA1 = 1;
	
	if(__DEBUGGING)
		init_uart();

}

void vn_tare()
{
	unsigned char i;
	_VN_SEL();
	xmit_spi(0x05);
	for(i = 0 ; i < 7;i++)
		xmit_spi(0x00);	
	_VN_DSEL();
	__delay_us(120);
	_VN_SEL();
	for(i = 0 ; i < 8;i++)
		xmit_spi(0x00);	
	_VN_DSEL();
}

void write_us_data()
{
	BYTE res;
	unsigned int b_len,b_len_written,msec;
	unsigned char i,num_samples;
	unsigned char buffer[15];
	US_SAMPLE *sample;
	BYTE sec,min;
	res = f_open(&file, "0:us_data.csv", FA_WRITE);
	if(res)
	{
		printf("error in opening us_data.csv\r\n");
		put_rc(res); 
		R_LED = 1;
		while(1)
		 _DI
	} 	 
	res = f_lseek(&file,file.fsize);
	if(res)
	{
		printf("error in moving r/w ptr us_data.csv\r\n");
		put_rc(res); 
		R_LED = 1;
		while(1)
		 _DI
	} 
	num_samples = us_get_sample_count();
	for(i = 0; i < num_samples;i++)
	{
		sample = us_get_sample_buffer();
		
		msec = sample->time >> 10;
		sec = sample->time >> 8;
		min = sample->time >> 8;
		//"t,,dis"
		sprintf(buffer,"%2d:%2d:%3d,,%s\r\n",min,sec,msec,sample->dist);
		b_len = strlen(buffer);
		res = f_write (&file,buffer, b_len, &b_len_written);
		if(b_len != b_len_written || res)//problem in writing
		{
			printf("error in writing array to us_data.csv stuck @:%u\r\n",i);
			put_rc(res);
			R_LED = 1;
			while(1)
			_DI
		}
		res = f_sync(&file);
		if(res)
		{
			printf("error in syncing file us_data stuck @:%u\r\n",i);
			put_rc(res);
			R_LED = 1;
			while(1)
			_DI
		}
		sample++;
		
	}
	res = f_close(&file);
	if(res)
	{
		printf("error in closing us_data.csv\r\n");
		put_rc(res); 
		R_LED = 1;
		while(1)
		 _DI
	} 
	

}

void write_gps_data()
{
	BYTE res;
	unsigned int b_len,b_len_written,msec;
	unsigned char i,num_samples;
	unsigned char buffer[80];
	GPS_SAMPLE *sample;
	BYTE sec,min;
	res = f_open(&file, "0:gps_data.csv", FA_WRITE);
	if(res)
	{
		printf("error in opening gps_data.csv\r\n");
		put_rc(res); 
		R_LED = 1;
		while(1)
		 _DI
	} 	 
	res = f_lseek(&file,file.fsize);
	if(res)
	{
		printf("error in moving r/w ptr\r\n");
		put_rc(res); 
		R_LED = 1;
		while(1)
		 _DI
	} 
	num_samples = gps_get_sample_count();
	for(i = 0; i < num_samples;i++)
	{
		sample = gps_get_sample_buffer();
		
		msec = sample->time >> 10;
		sec = sample->time >> 8;
		min = sample->time >> 8;
		//"t,,longitude,,latitude,,velocity,,heading"
		sprintf(buffer,"%2d:%2d:%3d,,%s,,%s,,%s,,%s\r\n",min,sec,msec,sample->longitude,
												sample->latitude,sample->velocity,
												sample->heading);
		b_len = strlen(buffer);
		res = f_write (&file,buffer, b_len, &b_len_written);
		if(b_len != b_len_written || res)//problem in writing
		{
			printf("error in writing array to gps_data.csv stuck @:%u\r\n",i);
			put_rc(res);
			R_LED = 1;
			while(1)
			_DI
		}
		res = f_sync(&file);
		if(res)
		{
			printf("error in syncing file gps_data stuck @:%u\r\n",i);
			put_rc(res);
			R_LED = 1;
			while(1)
			_DI
		}
		sample++;
		
	}
	res = f_close(&file);
	if(res)
	{
		printf("error in closing imu_data.csv\r\n");
		put_rc(res); 
		R_LED = 1;
		while(1)
		 _DI
	} 
	

}
void write_imu_data()
{
	BYTE res;
	unsigned int b_len,b_len_written,msec;
	unsigned char i,count;
	IMU_SAMPLE *imu_sample;
	BYTE sec,min;
	unsigned char buffer[150];
	
	res = f_open(&file, "0:imu_data.csv", FA_WRITE);
	if(res)
	{
		printf("error in opening imu_data.csv\r\n");
		put_rc(res); 
		R_LED = 1;
		while(1)
		 _DI
	} 	 
	res = f_lseek(&file,file.fsize);
	if(res)
	{
		printf("error in moving r/w ptr\r\n");
		put_rc(res); 
		R_LED = 1;
		while(1)
		 _DI
	} 	 
	
	count = imu_get_sample_count();
	for(i = 0;i< count;i++)
	{
		imu_sample = imu_get_sample_buffer();
		msec = imu_sample->time >> 10;
		sec = imu_sample->time >> 8;
		min = imu_sample->time >> 8;
		
		sprintf(buffer,"%2d:%2d:%3d,,%+07.2f,%+07.2f,%+07.2f,,%+07.4f,%+07.4f,%+07.4f,,%+07.3f,%+07.3f,%+07.3f,,%+08.4f,%+08.4f,%+08.4f\r\b",
		min,sec,msec,imu_sample->ypr[0],imu_sample->ypr[1],imu_sample->ypr[2]
		,imu_sample->mag[0],imu_sample->mag[1],imu_sample->mag[2]
		,imu_sample->acc[0],imu_sample->acc[1],imu_sample->acc[2]
		,imu_sample->rates[0],imu_sample->rates[1],imu_sample->rates[2]);
		
		b_len = strlen(buffer);
		res = f_write (&file, buffer, b_len, &b_len_written);
		if(b_len != b_len_written || res)//problem in writing
		{
			printf("error in writing array to imu_data.csv stuck @:%u\r\n",i);
			put_rc(res);
			R_LED = 1;
			while(1)
			_DI
		}
		res = f_sync(&file);
		if(res)
		{
			printf("error in syncing file stuck @:%u\r\n",i);
			put_rc(res);
			R_LED = 1;
			while(1)
			_DI
		}
		imu_sample++;
	}

	res = f_close(&file);
	if(res)
	{
		printf("error in closing imu_data.csv\r\n");
		put_rc(res); 
		R_LED = 1;
		while(1)
		 _DI
	} 
}

void prepair_files()
{
	unsigned int b_len,b_len_written;
	char temp_buffer[40];
	unsigned char i;
	BYTE res;
	init_spi_SD();
	/*
	prepair imu data file
	*/
	res = f_open(&file, "0:imu_data.csv", FA_CREATE_ALWAYS|FA_WRITE);
	if(res)
	{
		put_rc(res); 
		R_LED = 1;
		while(1)
		 _DI
	}
	sprintf(temp_buffer,"t,,Y,P,R,,mX,mY,mZ,,aX,aY,aZ,,Y,P,R\r\n");	
	b_len = strlen(temp_buffer);
	f_write (&file, temp_buffer, b_len, &b_len_written);
	if(b_len != b_len_written)
	{
		R_LED = 1;
		while(1)
		 _DI
	}
	f_close(&file);
	memset(temp_buffer,0,40);
	/*
	prepair gps data file
	*/
	res = f_open(&file, "0:gps_data.csv", FA_OPEN_ALWAYS | FA_WRITE);
	if(res)
	{
		put_rc(res); 
		R_LED = 1;
		while(1)
		 _DI
	}
	sprintf(temp_buffer,"t,,longitude,,latitude,,velocity,,heading\r\n");	
	b_len = strlen(temp_buffer);
	f_write (&file, temp_buffer, b_len, &b_len_written);
	if(b_len != b_len_written)
	{
		R_LED = 1;
		while(1)
		 _DI
	}
	f_close(&file);
	memset(temp_buffer,0,40);
	/*
	prepair rc rx data file
	*/
	res = f_open(&file, "0:rc_data.csv", FA_OPEN_ALWAYS | FA_WRITE);
	if(res)
	{
		put_rc(res); 
		R_LED = 1;
		while(1)
		 _DI
	}
	sprintf(temp_buffer,"t,,ch1,,ch2,,ch3,,ch4\r\n");	
	b_len = strlen(temp_buffer);
	f_write (&file, temp_buffer, b_len, &b_len_written);
	if(b_len != b_len_written)
	{
		R_LED = 1;
		while(1)
		 _DI
	}
	f_close(&file);
	memset(temp_buffer,0,40);
	/*
	prepair ultrasonic data file
	*/
	res = f_open(&file, "0:us_data.csv", FA_OPEN_ALWAYS | FA_WRITE);
	if(res)
	{
		put_rc(res); 
		R_LED = 1;
		while(1)
		 _DI
	}
	sprintf(temp_buffer,"t,,dist\r\n");	
	b_len = strlen(temp_buffer);
	f_write (&file, temp_buffer, b_len, &b_len_written);
	if(b_len != b_len_written)
	{
		R_LED = 1;
		while(1)
		 _DI
	}
	f_close(&file);
}
int main(void)
{

	FATFS Fs; 
	BYTE res;
	unsigned int x;


		unsigned int b_len,b_len_written;
		char temp_buffer[40];
		unsigned char i;
		
	__delay_ms(100);//wait for PLL to lock
	
	
	IoInit();
	init_gps();
	while(1);
	init_spi_SD();

	printf("initilizing...\r\n");
	if(RCON & 0x8000 || RCON & 0x4000)//TRAP or Illegal opcode
	{
		printf("fatal fault, RCON: %4X\r\n",RCON);
		while(1);
	}
	RCON = 0;
	
	res = 0;
	x = 0;
	

	res = disk_initialize(0);
	if(res)
	{

		if(__DEBUGGING)
			put_rc(res); 
		R_LED = 1;
		while(1);
	}	
	else
	{
		if(__DEBUGGING)
			printf("[+] Disk init Ok\r\n");
		res = f_mount(0, &Fs);
		if(__DEBUGGING)
		{
			if(res)
		 		put_rc(res); 
				printf("[+] Mount Ok\r\n");
		}
		
	}
	
	prepair_files();
	
	printf("begin....\r\n");
	
	init_spi_VN();
	time_to_write_imu = 0;
	init_ext_int();
	
/*	res = f_open(&file, "0:test.txt", FA_OPEN_ALWAYS|FA_WRITE);
	if(res)
	{
		printf("cannot open file...\r\n");
		put_rc(res); 
		R_LED = 1;
		while(1)
		 _DI
	}
	res = f_lseek(&file,file.fsize);
	if(res)
	{
		printf("error in moving r/w ptr\r\n");
		put_rc(res); 
		R_LED = 1;
		while(1)
		 _DI
	} 	
	while(1)
	{
		sprintf(temp_buffer,"(blah)current x = %5d\r\n",x++);	
		b_len = strlen(temp_buffer);
		res = f_write (&F_IMU, temp_buffer, b_len, &b_len_written);
		if(b_len != b_len_written || res)
		{
			printf("cannot write file...\r\n");
			R_LED = 1;
			while(1)
			 _DI
		}
		res = f_sync(&file);
		if(res)
		{
			printf("error in syncing file stuck @:%u\r\n",i);
			put_rc(res);
			R_LED = 1;
			while(1)
			_DI
		}
		printf("x=%d\r\n",x);
	}*/
	time_to_write_imu=0;
	time_to_write_gps=0;
	time_to_write_us=0;
	while(1)
	{
		if(time_to_write_imu)//runs every 75ms
		{
			write_imu_data();
			
			time_to_write_imu=0;
		}
		if(time_to_write_gps)//runs every 75ms
		{
			write_gps_data();
			
			time_to_write_gps=0;
		}
		if(time_to_write_us)//runs every 75ms
		{
			write_us_data();
			
			time_to_write_us=0;
		}
	
	}
		
}