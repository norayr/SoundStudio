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
	audio_info_t aud,aud2;
	int fd,err,samples;
	char arg[5];
	char *ptr;
	
	if ((fd=open("/dev/audioctl", O_RDONLY,0))== -1)
        {
        	printf("Open Failure\n");
        	exit(-1);
        }
        err=ioctl(fd,AUDIO_GETINFO,&aud);
        close(fd);
        
	while (1)
	{
		
		scanf("%s",&arg);
		ptr = arg;
		
		switch (*ptr++)
		{
			case ('P'): 
			switch (*ptr++)
			{
				case ('L'):
				aud.play.gain=atoi(ptr);
				fd=open("/dev/audioctl", O_RDONLY,0);
				err=ioctl(fd,AUDIO_GETINFO,aud2);
				aud.play.samples=aud2.play.samples;
				err=ioctl(fd,AUDIO_SETINFO,aud);
				close(fd);
				break;	
				case ('B'):
				aud.play.balance=atoi(ptr);
				fd=open("/dev/audioctl", O_RDONLY,0);
				err=ioctl(fd,AUDIO_SETINFO,aud);
				close(fd);
				break;	
				case ('P'):
				aud.play.port=atoi(ptr);
				fd=open("/dev/audioctl", O_RDONLY,0);
				err=ioctl(fd,AUDIO_SETINFO,aud);
				close(fd);
				break;	
					
			}
			break;	
			case ('R'): 
			switch (*ptr++)
			{
				case ('L'):
				aud.record.gain=atoi(ptr);
				fd=open("/dev/audioctl", O_RDONLY,0);
				err=ioctl(fd,AUDIO_SETINFO,aud);
				close(fd);
				break;	
				case ('B'):
				aud.record.balance=atoi(ptr);
				fd=open("/dev/audioctl", O_RDONLY,0);
				err=ioctl(fd,AUDIO_SETINFO,aud);
				close(fd);
				break;	
				case ('P'):
				aud.record.port=atoi(ptr);
				fd=open("/dev/audioctl", O_RDONLY,0);
				err=ioctl(fd,AUDIO_SETINFO,aud);
				close(fd);
				break;		
			}
			break;
		}		
			
	
	}

	return(0);
}