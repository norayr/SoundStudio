#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <math.h>

#include "s_tool.h"


int main (void)
{
	int fd,err;
	audio_info_t aud;
	
	if ((fd=open("/dev/audioctl", O_RDONLY,0))== -1)
        {
        	printf("Open Failure\n");
        	exit(-1);
        }
        
        err=ioctl(fd,AUDIO_GETINFO,&aud);

        printf("!%d %d %d",aud.play.gain,aud.play.balance,aud.play.port);
     
        
        printf("!%d %d %d!\n",aud.record.gain,aud.record.balance,aud.record.port);
    

	close (fd);

	return(0);
}