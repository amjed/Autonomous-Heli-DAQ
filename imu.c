//imu
#include "vn-100/inc/VN100.h"
#include "proj_consts.h"
#include "p24FJ64GA002.h"
#include "imu.h"

IMU_SAMPLE VNRRG[IMU_SAMPLE_SIZE];
unsigned char VNRRG_index = 0;
IMU_SAMPLE VNRRG2[IMU_SAMPLE_SIZE];
unsigned char VNRRG2_index = 0;
unsigned char VNRRG_buff_x = 0;

IMU_SAMPLE * imu_get_sample_buffer()
{
	if(!VNRRG_buff_x)
		return &VNRRG;
	else
		return &VNRRG2;
}

unsigned char imu_get_sample_count()
{
	return (unsigned char)IMU_SAMPLE_SIZE;
}
void __attribute__((interrupt,no_auto_psv)) _INT1Interrupt(void)
{
/*
	 ping-pong buffering is used in this		
	 one buffer is used to store the stream of data while
	 the other is being off-loaded to the SD card. and the
	 buffers are fliped & process is continued
	 
	 buffer will be full in 5ms * 20 = 100 ms
*/
	
	_INT1IF = 0;
	
	
	if(!VNRRG_buff_x)//if primery buffer VNRRG is currently the active buffer
	{
		if(VNRRG_index < IMU_SAMPLE_SIZE)//make sure not to overflow
		{
			VN100_SPI_GetYPRMagAccRates(0, VNRRG[VNRRG_index].ypr, VNRRG[VNRRG_index].mag, VNRRG[VNRRG_index].acc, VNRRG[VNRRG_index].rates);//read sensor
			VNRRG[VNRRG_index].time = get_time_word();
			VNRRG_index++;
		}
		else//if our main buffer is full
		{
			VNRRG_buff_x = 1;//switch to second buffer
			VNRRG2_index = 0;//reset secondery buffer index
			//store the read data secondery buffer
			VN100_SPI_GetYPRMagAccRates(0, VNRRG2[VNRRG2_index].ypr, VNRRG2[VNRRG2_index].mag, VNRRG2[VNRRG2_index].acc, VNRRG2[VNRRG2_index].rates);
			VNRRG2[VNRRG2_index].time = get_time_word();
			time_to_write_imu = 1;//write first buffer to SD card
			VNRRG2_index++;
			//next read will be stored in secondery
		}	
	}
	else
	{
		if(VNRRG2_index < IMU_SAMPLE_SIZE)//make sure not to overflow
		{
			VN100_SPI_GetYPRMagAccRates(0, VNRRG2[VNRRG2_index].ypr, VNRRG2[VNRRG2_index].mag, VNRRG2[VNRRG2_index].acc, VNRRG2[VNRRG2_index].rates);
			VNRRG2[VNRRG2_index].time = get_time_word();
			VNRRG2_index++;
		}
		else
		{
			VNRRG_buff_x = 0;//switch to primery buffer
			VNRRG_index = 0;//reset primery buffer index
			//store the read data secondery buffer
			VN100_SPI_GetYPRMagAccRates(0, VNRRG[VNRRG_index].ypr, VNRRG[VNRRG_index].mag, VNRRG[VNRRG_index].acc, VNRRG[VNRRG_index].rates);//read sensor
			VNRRG[VNRRG_index].time = get_time_word();
			VNRRG_index++;
			time_to_write_imu = 1;//write secondery buffer to SD card
			//next read will be stored in primery
		}
	}
	
}