#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <math.h>

#include "s_tool.h"

int fd;

int main (void)
{
	int err, vol;
	audio_info_t aud;
	
	if ((fd=open("/dev/audioctl", O_RDWR,0))== -1)
        {
        	printf("Open Failure\n");
        	exit(-1);
        }

	err = ioctl(fd,AUDIO_GETINFO,&aud);
	vol = aud.play.gain;

        aud.play.gain = 0;
	err = ioctl(fd,AUDIO_SETINFO,&aud);
	
        err = ioctl(fd,AUDIO_GETINFO,&aud);
        
        while (aud.play.active)
        {
        	err = ioctl(fd,AUDIO_GETINFO,&aud);	
        }
        
        aud.play.gain = vol;
	err = ioctl(fd,AUDIO_SETINFO,&aud);
	
	close (fd);

	return(0);
}