#ifndef __IMU_H
#define __IMU_H

#define IMU_SAMPLE_SIZE (unsigned char)		20 //size of ping-pong buffer to hold vn-100 samples


typedef struct __IMU_SAMPLE
{
	unsigned long time;
	float ypr[3];
	float mag[3];
	float acc[3];
	float rates[3];

}IMU_SAMPLE;

volatile extern unsigned char time_to_write_imu;

IMU_SAMPLE * imu_get_sample_buffer();
unsigned char imu_get_sample_count();
#endif
