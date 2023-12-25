#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <math.h>
#include <sys/shm.h>
#include <sys/stat.h>

#include "s_tool.h"

int main (void)
{
	int fd, fd2, soundcard_active = 1,err;
	struct stat buf;
	audio_info_t aud;
	off_t size;
	long rob=0;
	
	if ((fd=open("file.wav",O_RDONLY,0))==-1) 
        {
                fprintf(stderr,"file open failed\n");
                exit(-1);
        }
	if ((fd2=open("/dev/audioctl",O_RDONLY,0))==-1) 
        {
                fprintf(stderr,"failure");
                exit(-1);
        }
        ioctl(fd2,AUDIO_GETINFO,&aud);
        
      
	/*aud.record.buffer_size=8;
	ioctl(fd2,AUDIO_SETINFO,&aud);*/
	printf("%d\n",aud.record.buffer_size);
	
        fstat(fd,&buf);        
        while(soundcard_active)
        {
        	ioctl(fd2,AUDIO_GETINFO,&aud);
        	soundcard_active=aud.record.active;
        	rob = buf.st_size;
        	fstat(fd,&buf);
        	if (rob != buf.st_size) printf("%d\n",buf.st_size);
        	/*if (rob != buf.st_size) printf("%d\n",buf.st_size-rob);*/
        }
        close(fd);
        close(fd2);
	return(0);
}