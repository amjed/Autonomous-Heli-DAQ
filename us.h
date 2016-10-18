//ultrasonic
#ifndef _ULTRA_SONIC
#define _ULTRA_SONIC

#define US_SAMPLE_SIZE (unsigned char)		10 //size of ping-pong buffer to hold ultrasonic distance samples

typedef struct __US_SAMPLE
{
	unsigned long time;
	unsigned char dist[4];
}US_SAMPLE;

volatile extern unsigned char time_to_write_us;

US_SAMPLE * us_get_sample_buffer();
unsigned char us_get_sample_count();
#endif