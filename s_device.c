
#include <unistd.h>
#include <fcntl.h>
#include </usr/include/sys/ioctl.h>
#include </usr/include/sys/audioio.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>




#define DEV_PATH "/dev/audio"

int fd;

int main(int argc, char *argv[])
{
   
   audio_device_t  a1;
   int err;
   
   
 
   if ((fd=open(DEV_PATH, O_RDONLY,0))== -1)
   {
       printf("Failure\n");
       exit(-1);
   }
   
   err = ioctl (fd, AUDIO_GETDEV,&a1);
   printf ("%s\n",a1.name);
   close (fd);
   return(0);
}  