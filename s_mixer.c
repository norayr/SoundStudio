/* Mixer Manipulation routines	*/
/* a command line program designed specifically for use with TCL */
/* A part of the SharpeSound Studio 	*/
/* By Paul D. Sharpe			*/
/* 3rd Year Project university of Leeds		*/
/* revision:1			*/
/* Date: 27/2/95		*/

#include <unistd.h>
#include <fcntl.h>
#include <sys/soundcard.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define DSP_NAME "/dev/mixer"
#define MIX_GET	-1
#define LEFT(x) 	x & 0xff
#define RIGHT(x)	(x & 0xff00)>>8
#define VOLUME(l,r)	(l & 0xff) | ((r & 0xff)<<8)


int reccard (void);
int mixer_vol(int fd,int devnum,int GETSET_FLAG );
int rec_source(int fd,int devnum,int GETSET_FLAG);
int all_mix_sources(int fd);
void list_mask (int mask);
int all_rec_sources(int fd);
int all_stereo_sources(int fd);
int all_outputs(int fd);

int main(int argc, char *argv[])
{
  char *labels[]=SOUND_DEVICE_LABELS;
  int i,fd,dev;
  int volume ,left,right,mask;
  left = 100;
  right = 100;
 

  if ((fd=reccard())==0) {
	fprintf(stderr,"%s  error: Sound Card is unavailable",argv[0]);
	exit (-1);
  }
  if (argc>2)  {
	if (!strcmp(argv[1],"get") ) {

	   /* Get Volume of channels */
	   if (!strcmp(argv[2],"vol") ) {
		if (argc >=4) {
			dev=atoi(argv[3]);
		   	if ((volume=mixer_vol(fd,dev,MIX_GET))!=-1) {
			  printf(" %d %d ",LEFT(volume),RIGHT(volume));
			} 
		}
			  /* List all channels if none specified */
		else {  
			for (i=0;i<SOUND_MIXER_NRDEVICES;i++) {
			  if ((volume=mixer_vol(fd,i,MIX_GET))!=-1) {
			     printf("{ %d %d }",
					LEFT(volume),RIGHT(volume));
			  }
			}  
		}
 	   }
	   else if (!strcmp(argv[2],"dev") ) {
		if (argc >=4) {
		   if (!strcmp(argv[3],"all") ) {
			if ((mask=all_mix_sources(fd))!=-1) {
				list_mask(mask);
			}

		   }
		   if (!strcmp(argv[3],"rec") ) {
			if ((mask=(all_rec_sources(fd) & all_mix_sources(fd)))!=-1) {
				list_mask(mask);
			}

		   }
		   if (!strcmp(argv[3],"stereo") ) {
			if ((mask=(all_stereo_sources(fd) & all_mix_sources(fd)))!=-1) {
				list_mask(mask);
			}

		   }
		   if (!strcmp(argv[3],"labels") ) {
			for (i=0;i<SOUND_MIXER_NRDEVICES;i++) {
				printf ("%s ",labels[i]);
			}

		   }
		   if (!strcmp(argv[3],"out") ) {
			 if ((mask=(all_outputs(fd) & all_mix_sources(fd)))!=-1) {
				list_mask(mask);
			}

		   }
		} 
		else {
		   	/* Do the same as "all" */
		   	if ((mask=all_mix_sources(fd))!=-1) {
				list_mask(mask);
			}
		} 
    	   }
	   else if (!strcmp(argv[2],"src") ) {
		if (argc >=4) {
		/* Active status of a single device */
			dev=atoi(argv[3]);
			if ((mask=rec_source(fd,dev,MIX_GET))!=-1) {
				printf("%d",mask);	
			}
		     }
		else {   
			if ((mask=rec_source(fd,MIX_GET,MIX_GET))!=-1) {
				list_mask(mask);
			}
		}
	   }
           else {
		fprintf (stderr,"%s  get: Invalid option\n",argv[0]);
		exit(-1);	
	   }
    	}/* get finish */

	else if (!strcmp(argv[1],"set") ) {
	   /* set volume */
	   if (!strcmp(argv[2],"vol") ) {
		if (argc >=5) {
			dev=atoi(argv[3]);
			left= atoi(argv[4]);
			right=atoi(argv[5]);
			  
		   	if ((volume=mixer_vol(fd,dev,VOLUME(left,right)))!=-1) {
			  	printf("%d %d",
					LEFT(volume),RIGHT(volume));
			} 
		}
	   }
	   else if (!strcmp(argv[2],"src") ) {
	       if (argc >=5) {
			dev=atoi(argv[3]);
			/*boolean to set or unset */
			i= atoi(argv[4]);
	  
		   	if ((mask=rec_source(fd,dev,i))!=-1) {
			list_mask(mask);
			} 
		}



	   } 
	   else {
		fprintf (stderr,"%s  set: Invalid option\n",argv[0]);
		exit(-1);
	   }
	} /* set finish */
 
  } 
  else {
	fprintf (stderr,"%s : Insufficient arguments\n",argv[0]);
	exit (-1);
  }
	
  close (fd);
  return (0);
}	

/* rec_source
# This sets up and reads the active recording sources
# PARAMETERS:
# fd - file descriptor to open mixer device.
# devnum - the number describing the mixer channel or MIX_GET
#    if the complete mask of devices is wanted.
# GETSET_FLAG -   MIX_GET to get current status.
# or 1 to make active device or 0 to switch off.
# returns -1 if failed, a boolean value if the status of a specific device 
# is requested or the mask of active devices if otherwise successful.
*/
int rec_source(int fd,int devnum,int GETSET_FLAG)
{ 
  int recdev=-1 ; 

  /* read mask of current active devices */   
  if (ioctl(fd, SOUND_MIXER_READ_RECSRC,&recdev)==-1) { 
		fprintf (stderr,"Error: can't read active recording sources of mixer");
		return(-1);
	}
  if ( ((devnum<SOUND_MIXER_NRDEVICES) && (devnum >= 0))) {

	/* Set the device to be active */
	if (GETSET_FLAG!=MIX_GET) {

		/* insert request for active device */
		if (GETSET_FLAG) {
			recdev=recdev | (1<<devnum);
		} 
		else {
			recdev=recdev ^ (1<<devnum);
		}

		/* make request to card */
		if (ioctl(fd, SOUND_MIXER_WRITE_RECSRC,&recdev)==-1) { 
			fprintf (stderr,"Error: can't write active recording sources to mixer");
			return(-1);
		}

	    	/*Get the new bit mask of active sources */
		recdev=rec_source(fd,MIX_GET,MIX_GET);
	} 
	else {
		/* Get status of requested device (boolean) */
		recdev= ((recdev>>devnum) & 1);
	}
  } 
  else 
  {
	/* The mask containing the recording sources will be returned*/
  } 
  return (recdev);
}

/* listmask */
/* This prints to stdout the mask such as is returned 
   from mixer ioctl calls.
   Output is simple to be used in a tcl list.
*/
void list_mask (int mask)
{
	int i;

	for (i=0;i<SOUND_MIXER_NRDEVICES;i++)
		if (mask & (1<<i)) 
		{
		    printf("%d ",i);
		} 
	printf("\n");
}


/* all_rec_sources */
/* returns a bit mask indicating the devices capabible of recording*/
int all_rec_sources(int fd)
{
	int map=0;

 	if (ioctl(fd,SOUND_MIXER_READ_RECMASK,&map)==-1 )
		{	fprintf(stderr,"Error:No Recording Devices Present");
			return (-1);
		}
	return(map);
}

/* all_outputs */
/* returns a bit mask indicating the device that are not recording */
int all_outputs(int fd) 
{
	int allmask=0, recmask=0,newmask=0;
	allmask= all_mix_sources(fd);
	recmask=all_rec_sources(fd);
	if ((allmask!=-1)&&(recmask!=-1)) {
		newmask=allmask ^ recmask;
		return (newmask);
	}
	else {
		return(-1);
	}	 

}


/* all_mix_sources */
/* returns a bit mask indicating the devices of the mixer*/
int all_mix_sources(int fd)
{
	int map=0;

 	if (ioctl(fd,SOUND_MIXER_READ_DEVMASK,&map)==-1 )
		{	fprintf(stderr,"Error:No Mixer Present");
			return (-1);
		}
	return(map);
}

/* all_stereo_sources */
/* returns a bit mask indicating the stereo (2-channel) devices of the mixer*/
int all_stereo_sources(int fd)
{
	int map=0;

 	if (ioctl(fd,SOUND_MIXER_READ_STEREODEVS,&map)==-1 )
		{	fprintf(stderr,"Error: No Stereo Devices Present");
			return (-1);
		}
	return(map);
}

/* mixer_vol
# This sets and reads the volume of an individual mixer channel.
# PARAMETERS:
# fd - file descriptor to open mixer device.
# devnum - the number describing the mixer channel.
# GETSET_FLAG - if set to MIX_GET simply reads the current volume,
#		otherwise it shuld be a valid volume.
#		The macro VOLUME(l,r) may be used here to convert 
#		from channel volumes (range:0-100) to volume required.*/
int mixer_vol(int fd,int devnum,int GETSET_FLAG )
{
	int volume=GETSET_FLAG;
	if ( (devnum<SOUND_MIXER_NRDEVICES) && (devnum >= 0) ) {
	    if (volume>=0 && (LEFT(volume))<100 && (RIGHT(volume))<100) {
	        if (ioctl(fd, MIXER_WRITE(devnum),&volume)==-1) { 
			fprintf (stderr,"Error: Can't write volume of device %d", devnum);
			exit(-1);
		}
	    
	    }	
	    if (ioctl(fd, MIXER_READ(devnum),&volume)==-1) { 
			fprintf (stderr,"Error: Can't read volume of device %d\n", devnum);
			return(-1);
	    }
	    return (volume);
	}
	else {
		return(-1);
	}
}



/* Reccard() */
/* This procedure simply opens the Mixer device for reading and writing*/
int reccard (void)

{
	int fd;
		if ((fd=open(DSP_NAME,O_RDWR,0))==-1) 
		 {
			/*printf("open failed");*/
			return(0);
		 }
	 
return (fd);
	
}
