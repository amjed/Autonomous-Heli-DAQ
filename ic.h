//inputcapture
#ifndef _INPUT_CAP
#define _INPUT_CAP



#define IC_SAMPLE_SIZE (unsigned char)		3

typedef struct __IC_SAMPLE
{
	unsigned long time;
	unsigned int ic1;
	unsigned int ic2;
	unsigned int ic3;
	unsigned int ic4;

}IC_SAMPLE;


unsigned int get_pulse_time(unsigned char ch);
void init_input_capture();
volatile extern unsigned char time_to_write_ic;
#endif