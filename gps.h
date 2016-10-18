#ifndef __GPS_H
#define __GPS_H


#define GPS_SAMPLE_SIZE (unsigned char)		3

typedef struct __GPS_SAMPLE
{
	unsigned long time;
	unsigned char heading[7];
	unsigned char velocity[6];
	unsigned char longitude[10];
	unsigned char latitude[9];

}GPS_SAMPLE;


volatile unsigned char extern time_to_write_gps;
GPS_SAMPLE*	gps_get_sample_buffer();
unsigned char gps_get_sample_count();
void init_gps();
#endif
