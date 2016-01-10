
#include <stdio.h>
#include <malloc.h>
#include <dpmi.h>
#include <math.h>
#include "wss.h"

#define AMP1	(double)(0.25 * 32767)
#define AMP2	(double)(0.50 * 32767)
#define FREQ1 	(double)(1000.00)
#define FREQ2 	(double)(440.00)
#define LATENCY	(float)(0.025)

static DWORD latency_size;

static int write(short * buffer, int len)
{
	int samples = w_get_buffer_size() - w_get_latency() - latency_size;
	if((samples <= 0) || (len == 0))
		return 0;
	if(len > samples)
		len = samples;
	w_lock_mixing_buffer(len);
	w_mixing_stereo(buffer, len, 256, 256);
	w_unlock_mixing_buffer();
	return len;
}
	
int main()
{
	
	DWORD smpr; 
	int frame, rest, offs, written, n;
	short * buff;
	
	w_set_device_master_volume(0);
	
	if(w_sound_device_init(28, 24096) == FALSE)
	{
		printf("%s\n", w_get_error_message());
		return -1;
	}
	
	smpr =  w_get_nominal_sample_rate();
	latency_size = (DWORD)(smpr * LATENCY);
	
	frame = 1 * smpr; //1 sec buffer 
	if((buff = (short *)malloc(frame*4))== NULL)	//16bit stereo buffer
	{
		w_sound_device_exit();
		printf("Not enough memory.\n");
		return -1;
	}
	
	n = 0;
	while(n < frame)
	{
		buff[n*2+0]= (short)(AMP1 * sin((FREQ1 * (2 * PI) * n) / smpr));
		buff[n*2+1]= (short)(AMP2 * sin((FREQ2 * (2 * PI) * n) / smpr));
		n += 1;
	}
	
	w_lock_mixing_buffer(latency_size);
	w_mixing_zero();
	w_unlock_mixing_buffer();
	
	offs = 0;
	rest = frame;
	while(!kbhit())
	{
		__dpmi_yield();
		if((written = write(&buff[offs*2], rest)) != 0)
		{
			if((rest = (rest - written)) == 0)
				rest = frame;
			offs = frame - rest;		
		}
	}	
	
	free(buff);
	
	w_sound_device_exit();
		
	return 0;
}