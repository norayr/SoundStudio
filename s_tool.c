/* A sound card application interface 	*/

/* valid options:			 */
/* diagnostics, play, stop, mixerset,rec */
/* prints to stdout for accessablilty to TCL/Tk*/
/* By Paul D. Sharpe 21/11/94 */

/* revision: 3 : 27/2/95 */

/* revision: 4 : 15/4/97 To remove "3 Minute Problem" */
/* Voxware driver v.2.4 */
/* This revision is necessary since the card must be set upon playing
   and recording, not separately. Thus rec and play both need the four
   arguments filename, bitrate, channel, samplrate. */

/* revision: 5 : 11/11/98 Sun Port  */
/* This now allows SoundStudio to be used on a SUN based unix machine.
   The system auto-detects the platform, and then selects the correct
   ioctl command accordingly. The ioctl commands for each platform are
   very different so when running on a linux platform, all the SUN
   commands had to be declared (as NULL - efectively dummy commands)so
   the compiler would recognise them, and the same when running on a
   SUN machine. Therefore there was an increase in the definition and
   header commands, which were put in a new header file: s_tool.h */
   
/* revision 6 : 15/6/99 Version 1.0.0alpha */
/* This now includes play-line, and VU-meter calculation on play,
   and VU meter calculation on record. Studio_tool can still be
   used from the command line, as before, and it will only output
   line and meter data if specifically requested ie by Studio*/
                               
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
  
#define MAX_BIT_RATE	16
#define MAX_CHANNELS	2
#define ST_SET		1
#define ST_GET		0

typedef int ST_CONFIG[3];

/* Global Variables */
int fd, pid = 0, pid2 = 0, pid3 = 0, global_decay, global_bits, global_channels, global_rate, commandline = 1;

/* Function Prototypes */
void diagnostics (void);
int *cardctl (int fd, ST_CONFIG parm, int st_flag);
int setcard (void);
int reccard (void);
int record (char *filename, int attacktime, int decaytime);
void equate_ST_CONFIG (ST_CONFIG a, ST_CONFIG b);
int play (char *filename, int samples, int pixels, int attacktime, int decaytime);
void Decay_It (int decay[global_channels], int final);
void sigquit();
void sigquit2();
void sigquit3();
void sigquit4();
int round (int i, int j);
int trunc (int i, int j);
void Final_Decay(void);
static char *VUdata = NULL;

int main(int argc, char *argv[])
{
	ST_CONFIG new, curr;
	int err, no_of_samples, pixels_in_plot,main_pid;
	int meter_attacktime,meter_decaytime;

        setlinebuf (stdout);
  
        if ((fd=open(DSP_NAME, O_RDONLY,0))== -1)
        {
        	printf("Failure\n");
          	exit(-1);
        }

        close(fd);
               
	if (argc>1)  
	{
		if (!strcmp(argv[1],"diagnostics")) 
		{
			diagnostics();
		}
 
	  	else if (!strcmp(argv[1],"play")) 
		{
			if (argc<3) 
			{ 
				fprintf (stderr,"%s play : Need filename",argv[0]); exit (-1);
			}
			
			if (!setcard()) 
			{
				fprintf (stderr,"%s  : Sound Card unavailable",argv[0]);
				exit (-1);
			}
		
			if (argc>=6) 
			{
				new[0]=atoi(argv[3]);
				new[1]=atoi(argv[4]);
				new[2]=atoi(argv[5]);
				global_bits = new[0];
				global_channels = new[1];
				global_rate = new[2];
				cardctl (fd,new,ST_SET);
			} 
			if (argc>=9) 
			{
				if (!strcmp(argv[6],"meter"))
				{
					meter_attacktime = atoi(argv[7]);
					meter_decaytime = atoi(argv[8]);
					global_decay = meter_decaytime;
					commandline = 0;
				}
			}
			else
			{
				meter_attacktime = 0;
				meter_decaytime = 0;
			}
			if (argc==12)
			{
				if (!strcmp(argv[9],"line"))
				{
					pixels_in_plot = atoi(argv[10]);
					no_of_samples = atoi(argv[11]);
                			commandline = 0;
				}
				
			}
			else
			{
				no_of_samples = 0;
				pixels_in_plot = 0;
				
			}
			if (!commandline)
			{
				main_pid = getpid();
                		printf("%d\n",main_pid);
			
			}
 			/* For debug purposes */
			/*puts("Set done ");*/
			
			cardctl (fd,curr,ST_GET);
		
			/*printf ("%d %d %d",curr[0],curr[1],curr[2]);*/

			/* Play the Sample */
			/*printf("commandline:%d\n",commandline);
			printf("playing %s %d %d %d %d\n", argv[2], no_of_samples, pixels_in_plot, meter_attacktime, meter_decaytime);*/
			play(argv[2], no_of_samples, pixels_in_plot, meter_attacktime, meter_decaytime);
		
		}

	 	else if (!strcmp(argv[1],"rec")) 
		{ 
			if (argc<3) 
			{ 
				fprintf (stderr,"%s rec : Need filename",argv[0]); exit (-1);
			}
			
			if (!reccard()) 
			{
				fprintf (stderr,"%s  : Sound Card unavailable",argv[0]);
				exit (-1);
			}
			
			if (argc>=6) 
			{
				new[0]=atoi(argv[3]);
				new[1]=atoi(argv[4]);
				new[2]=atoi(argv[5]);
				global_bits = new[0];
				global_channels = new[1];
				global_rate = new[2];
				cardctl (fd,new,ST_SET);
			}			
			if (argc==9) 
			{
				if (!strcmp(argv[6],"meter"))
				{
					meter_attacktime = atoi(argv[7]);
					meter_decaytime = atoi(argv[8]);
					global_decay = meter_decaytime;
					commandline = 0;
				}
			}
			else
			{
				meter_attacktime = 0;
				meter_decaytime = 0;
			}
			if (!commandline)
			{
				main_pid = getpid();
                		printf("%d\n",main_pid);
			
			}
			/* For debug purposes */
			/*puts("Set done ");
			cardctl (fd,curr,ST_GET);
			printf ("%d %d %d",curr[0],curr[1],curr[2]);*/
			
			
			record(argv[2], meter_attacktime, meter_decaytime);
		}
		
	  	else if (!strcmp(argv[1],"mixerset")) 
		{
			printf("mixerset");
		}
	}
	 
	else
	{
	 	printf ("%s:Too few arguments\n",argv[0]);
	}

	return(0);
}

int record (char *filename, int attacktime, int decaytime)
{
  char buffer[1024];
  int ffd, ffd2, next_count, l, err, bufsize, bytesread, i = 0, rounded_samplerepeat, 
    buffernumber = 0, samplesize, oldvalue = 0, newvalue = 0, outputnow = 0, optally = 0, 
    exitnow = 0, C_finishfinaldecay = 0, fragsize, op32, fd2, bitvalue[global_bits / 8], 
    chancnt, bitcnt, opcnt, C_chancnt, valuedB[global_channels],
    decay[global_channels];
# ifdef SunOS
  audio_info_t aud,aud1;
# endif
  count_info info;
  long int time;
  long bufstart = 0;  /* byte number of the beginning of the output
			 buffer */
  long bytesprocessed = 0; /* Running count of processed sound bytes */
  long lastbyte = 0;  /* count of number of bytes read (not processed)
			 so far */
  struct stat ffd2stat; /* For finding out how long the recorded file
			   is so far */
  int soundcaps; /* Real time and other capabilities of the sound card
		    fitted in this box */
  key_t VUmemid; 
  short op8,op16;
  float valuedBfloat[global_channels], rawsamplevalue[global_channels], value[global_channels];
  
  time = decaytime * 10000L;	
  bufsize = sizeof(buffer);
  samplesize = (global_bits / 8) * global_channels;
  
  if ((ffd=creat((char *)filename,0640))==-1) 
    {
      fprintf(stderr,"File create failed");
      exit(-1);
    }
  if ((ffd2=open((char *)filename,O_RDONLY,0))==-1) 
    {
      fprintf(stderr,"file open failed");
      exit(-1);
    }
  /* Open The Psuedo-device if the platform is Solaris */
# ifdef SunOS
      if ((fd2=open("/dev/audioctl", O_RDONLY,0))== -1)
	{
	  fprintf(stderr, "Read Failure\n");
	  exit(-1);
	}
# endif
# ifdef Linux
      ioctl(fd, SNDCTL_DSP_GETCAPS, &soundcaps);
# endif
  next_count = sizeof(buffer);
        
  /* Set-up the soundcard fragment size, as an aid to real-time
     calcualtions.  Top word is number of fragments; bottom is
     log_2 fragment size */

# ifdef Linux  
      if (samplesize == 1) fragsize = (0x7fff << 16) | (AUDIO_FRAGSIZE);
      if (samplesize == 2) fragsize = (0x7fff << 16) | (AUDIO_FRAGSIZE+1);
      if (samplesize == 4) fragsize = (0x7fff << 16) | (AUDIO_FRAGSIZE+2);
      if (samplesize == 8) fragsize = (0x7fff << 16) | (AUDIO_FRAGSIZE+3);
      if (samplesize == 16) fragsize = (0x7fff << 16) | (AUDIO_FRAGSIZE+4);
      ioctl(fd, SNDCTL_DSP_SETFRAGMENT, &fragsize);
      ioctl(fd, SNDCTL_DSP_RESET, NULL);
# endif
  /* Allocate a piece of shared memory */
  if ((VUmemid = shmget (IPC_PRIVATE, sizeof(SharedVUVariables),0660)) == -1)
    {
      fprintf(stderr,"Shared Memory Failure");
      exit (-1);
    }
  /* Attach the structure VUdata to the shared memory block */
  VUdata  = (char *)shmat (VUmemid,(char *)0,0);
  shmctl (VUmemid,IPC_RMID,NULL);
  
  for (opcnt=0; opcnt<4; ++opcnt) {
    ((SharedVUVariables *)VUdata)->sharedvaluedB[opcnt] = 0;
    ((SharedVUVariables *)VUdata)->shareddecay[opcnt] = 0;
  }
  ((SharedVUVariables *)VUdata)->sharednewvalue = 0;
  ((SharedVUVariables *)VUdata)->sharedexitnow = 0;
  ((SharedVUVariables *)VUdata)->sharedfinaldecaydone = 0;
  ((SharedVUVariables *)VUdata)->sharedmeterstarted = 0;
  
  bytesread = 1;
  newvalue = 0;
  
  rounded_samplerepeat = round ((global_rate * attacktime),1000);
  
  /* Do the first fork - the meter decay routine */
  if (pid = fork())
    {
      signal(SIGTERM, sigquit);
      for (C_chancnt = 0;C_chancnt < global_channels;C_chancnt++)
	decay[C_chancnt] = -100;
      while(!((SharedVUVariables *)VUdata)->sharedexitnow && !commandline)
	{
	  if (((SharedVUVariables *)VUdata)->sharednewvalue == 1)
	    {
	      for (C_chancnt = 0;C_chancnt <
		     global_channels;C_chancnt++)
		{
		  decay[C_chancnt] = ((SharedVUVariables
				       *)VUdata)->sharedvaluedB[C_chancnt];
		}
	      ((SharedVUVariables *)VUdata)->sharednewvalue = 0;
	    }
	  if (((SharedVUVariables *)VUdata)->sharedmeterstarted)
	    {
	      Decay_It (decay, 0);
	      usleep(time);
	    }
	}
      if (commandline == 0) Final_Decay();
      if (commandline == 1) while (!((SharedVUVariables *)VUdata)->sharedexitnow);
      kill (pid,SIGTERM);	
    }
  else
    {
      /* Do the second fork - the meter routine */
      if (pid2 = fork())
	{
	  signal(SIGTERM, sigquit2);		
	  while (bytesread  > 0 && !((SharedVUVariables *)VUdata)->sharedexitnow && !commandline)	
	    {
	      while (!outputnow)
		{
		  /* Get the value for the number of samples
		     processed.  We would like to use the hardware
		     pointer to determine the number of samples to
		     read, but unfortuantely, just because the sound
		     card has read them does not mean we can read them
		     back from the temprorary file yet!  So we use the
		     actual size of the temp file instead.  Bonus:
		     this is  also portable! */

		  oldvalue = newvalue;
		  usleep(5000);
		  fstat(ffd2, &ffd2stat);
		  newvalue = ffd2stat.st_size/samplesize;
		  
		  /* Output if no samples have been processed - ie it's all over! */
/*		  if (newvalue == 0) 
		    {
		      newvalue = oldvalue;
		      outputnow = 1;
		      ((SharedVUVariables *)VUdata)->sharedexitnow = 1;
		    }
*/
		  if (newvalue != oldvalue) 
		    {
		      /* Update the tally */
		      optally = optally + (newvalue - oldvalue);
		      
		      /* Output a value if the tally has breached the required value */
		      if (optally >= rounded_samplerepeat)
			{			
			  outputnow= 1;
			  /* Set the buffer index number to correspond to the required sample */
			  i = i + (optally * samplesize);

			  /* Reset the tally */
			  optally = 0;		
			} else {
			  usleep(2000);
			}
		    }
		}

	      while (outputnow)
		{
		  if (!((SharedVUVariables *)VUdata)->sharedexitnow)
		    {
		      /* Read and process vaules up to byte i */
		      double largest[global_channels];
		      for (chancnt = 0; chancnt < global_channels; chancnt++) 
			largest[chancnt] = 0.0;
		      while (bytesprocessed < i) {

			/* Read in the next buffer if the required
			/* sample is not in the current buffer */
			while (lastbyte == bytesprocessed) {
			  bufstart = lastbyte;
			  bytesread = read(ffd2,buffer,512);
			  lastbyte += bytesread;
			}
			/* If the required sample is in the current buffer, then read it in   */
			/* This way of doing this, is a bit kack-handed...feel free to change */
			/* it, as required						      */
			
			if (bytesread)
			  {
			    for (chancnt = 0; chancnt < global_channels; chancnt++)
			      {
				for (bitcnt = 0; bitcnt < (global_bits/8); bitcnt++)
				  bitvalue [bitcnt] = buffer[bytesprocessed - bufstart
							    + bitcnt + chancnt*(global_bits/8)];
				switch (global_bits) {
				case 8:
				  op8 = bitvalue[0];
				  if (op8 < 0) op8 += 256;
				  op8 -= 128;
				  if (op8 < 0) op8 = (~op8) - 1; 
				  if (op8 == 0) op8 = 1;
				  value[chancnt] = (1.0/127.0) * op8;
				  break;
				  
				case 16:
#                                 ifdef BigEndian
				    op16 = (bitvalue[0] << 8) + bitvalue[1];
#                                 else
				    op16 = (bitvalue[1] << 8) + bitvalue[0];
#                                 endif
				  if (op16 < 0) op16 = (~op16) - 1;
				  if (op16 == 0) op16 = 1;
				  value[chancnt] = (1.0/32767.0) * op16;
				  break;
				  
				case 32:
#                                 ifdef BigEndian
				    op32 = (bitvalue[0] << 24) + (bitvalue[1] << 16) + (bitvalue[2] << 8) + bitvalue[3];
#                                 else
				    op32 = (bitvalue[3] << 24) + (bitvalue[2] << 16) + (bitvalue[1] << 8) + bitvalue[0];
#                                 endif
				  if (op32 < 0) op32 = (~op32) - 1;
				  if (op32 == 0) op32 = 1;
				  value[chancnt] = (1.0/2147483647.0) * op32;
				  break;
				}
				if (value[chancnt] > largest[chancnt])
				  largest[chancnt] = value[chancnt];
			      }
			    bytesprocessed += samplesize;
			  }
		      }
		      /* Convert into dB and output */		     			
		      for (opcnt = 0; opcnt < global_channels; opcnt++)
			{
			  valuedBfloat[opcnt] = 20 * (log10 (largest [opcnt]));
			  valuedB[opcnt] = (int) rint(valuedBfloat[opcnt]);		     						
			  if (valuedB[opcnt] < ((SharedVUVariables *)VUdata)->shareddecay[opcnt] &&((SharedVUVariables *)VUdata)->sharednewvalue )
			    {
			      valuedB[opcnt] = ((SharedVUVariables *)VUdata)->shareddecay[opcnt];	
			    } 
			  ((SharedVUVariables *)VUdata)->sharedvaluedB[opcnt] = valuedB[opcnt];
			  ((SharedVUVariables *)VUdata)->sharedmeterstarted = 1;	     					
			  ((SharedVUVariables *)VUdata)->sharednewvalue = 1;
			}		 
		      outputnow = 0;
		    }
		}
	      usleep(50000);
	    }
	  while (!((SharedVUVariables *)VUdata)->sharedfinaldecaydone);
	  if (commandline == 1) while (!((SharedVUVariables *)VUdata)->sharedexitnow);
	  kill(pid2,SIGTERM);
	}	
      else
	{
	  /* Do the final fork - ie recording the actual soundfile */		
	  while ((l=read(fd,buffer,bufsize))>0)
	    {
	      if (write(ffd,buffer,l)==-1) 
		{
		  fprintf(stderr,"Can't Write to File");
		  exit(-1);
		}
	    
	    }
	  ((SharedVUVariables *)VUdata)->sharedexitnow = 1;
	  close (fd);
	  exit(0);						
	}	
      
    }
  /* Close the file, and release the shared memory block */		
  close (ffd);
  shmdt(VUdata);	
  return(0);    
}

int play (char *filename, int samples, int pixels, int attacktime, int decaytime)
{
  char buffer[16384];
  int ffd, ffd2, next_count, l, soundcard_active = 1, err, bufsize, sampletally = 0, 
    roundedsample, samplesread, overload, cntr = 1, line = 0, bytesread, i = 0, 
    rounded_samplerepeat, buffernumber, samplesize,oldvalue = 0, newvalue = 0, 
    outputnow = 0, optally = 0, exitnow = 0, C_finishfinaldecay = 0, fragsize, op32, 
    fd2, bitvalue[global_bits / 8], chancnt, bitcnt, opcnt, C_chancnt, repeat,
    valuedB[global_channels], decay[global_channels], pixeltally = 0;
# ifdef SunOS
  audio_info_t aud,aud1;
# endif
  count_info info;
  long int samples1 = 0, samples2 = 0, samplesold = 0, time;
  long bufstart = 0;  /* byte number of the beginning of the output
			 buffer */
  long bytesprocessed = 0; /* Running count of processed sound bytes */
  long lastbyte = 0;  /* count of number of bytes read (not processed)
			 so far */
  key_t VUmemid; 
  short op8,op16;
  float valuedBfloat[global_channels], rawsamplevalue[global_channels], correctiontally = 0, 
    correctionfactor, rawsample, value[global_channels];

  time = decaytime * 10000L;	
  bufsize = sizeof(buffer);
  samplesize = (global_bits / 8) * global_channels;
  
  if ((ffd=open((char *)filename,O_RDONLY,0))==-1) 
    {
      fprintf(stderr,"file open failed");
      exit(-1);
    }
  if ((ffd2=open((char *)filename,O_RDONLY,0))==-1) 
    {
      fprintf(stderr,"file open failed");
      exit(-1);
    }
  /* Open The Psuedo-device if the platform is Solaris */
# ifdef SunOS
      if ((fd2=open("/dev/audioctl", O_RDONLY,0))== -1)
	{
	  fprintf(stderr, "Read Failure\n");
	  exit(-1);
	}
# endif
  /* Setup the variables necessary for the calculation of the play-line */
  /* if the value of samples is greater than zero                       */
  if (samples != 0) 
    {
      rawsample = (float) samples / (float) pixels;
      line = 1;
      roundedsample = round (samples,pixels);
      correctionfactor = (float) roundedsample - rawsample;
      repeat = round (round((global_rate * pixels), samples), 50);
      if (repeat == 0) repeat = 1;
    }
  next_count = sizeof(buffer);
  
  /* Set-up the soundcard fragment size, as an aid to real-time
     calcualtions.  Top word is number of fragments; bottom is
     log_2 fragment size */
  
#  ifdef Linux
      if (samplesize == 1) fragsize = (8 << 16) | (AUDIO_FRAGSIZE);
      if (samplesize == 2) fragsize = (8 << 16) | (AUDIO_FRAGSIZE+1);
      if (samplesize == 4) fragsize = (8 << 16) | (AUDIO_FRAGSIZE+2);
      if (samplesize == 8) fragsize = (8 << 16) | (AUDIO_FRAGSIZE+3);
      if (samplesize == 16) fragsize = (8 << 16) | (AUDIO_FRAGSIZE+4);
      ioctl(fd, SNDCTL_DSP_SETFRAGMENT, &fragsize);
      ioctl(fd, SNDCTL_DSP_RESET, NULL);
#  endif
  /* Allocate a piece of shared memory */
  if ((VUmemid = shmget (IPC_PRIVATE, sizeof(SharedVUVariables),0660)) == -1)
    {
      fprintf(stderr,"Shared Memory Failure");
      exit (-1);
    }
  /* Attach the structure VUdata to the shared memory block */
  VUdata  = (char *)shmat (VUmemid,(char *)0,0);
  shmctl (VUmemid,IPC_RMID,NULL);
  for (opcnt=0; opcnt<4; ++opcnt) {
    ((SharedVUVariables *)VUdata)->sharedvaluedB[opcnt] = 0;
    ((SharedVUVariables *)VUdata)->shareddecay[opcnt] = 0;
  }
  ((SharedVUVariables *)VUdata)->sharednewvalue = 0;
  ((SharedVUVariables *)VUdata)->sharedexitnow = 0;
  ((SharedVUVariables *)VUdata)->sharedfinaldecaydone = 0;
  ((SharedVUVariables *)VUdata)->sharedmeterstarted = 0;
  
  bytesread = 1;
  newvalue = 0;
  
  rounded_samplerepeat = round ((global_rate * attacktime),1000);
  
  /* Do the first fork - the meter decay routine */
  if (pid = fork())
    {
      signal(SIGTERM, sigquit);
      for (C_chancnt = 0;C_chancnt < global_channels;C_chancnt++)
	decay[C_chancnt] = -100;
      while(!((SharedVUVariables *)VUdata)->sharedexitnow && !commandline)
	{
	  if (((SharedVUVariables *)VUdata)->sharednewvalue == 1)
	    {
	      for (C_chancnt = 0;C_chancnt < global_channels;C_chancnt++)
		{
		  decay[C_chancnt] = ((SharedVUVariables *)VUdata)->sharedvaluedB[C_chancnt];
		}
	      ((SharedVUVariables *)VUdata)->sharednewvalue = 0;
	    }
	  if (((SharedVUVariables *)VUdata)->sharedmeterstarted)
	    {
	      Decay_It (decay, 0);
	      usleep(time);
	    }
	}
      if (commandline == 0) Final_Decay();
      if (commandline == 1)
	  while (!((SharedVUVariables *)VUdata)->sharedexitnow)
	      sleep(1);
      kill (pid,SIGTERM);	
    }
  else 
    {
      /* Do the second fork - the meter routine */
      if (pid2 = fork())
	{
	  signal(SIGTERM, sigquit2);		
	  while (bytesread  > 0 && !((SharedVUVariables *)VUdata)->sharedexitnow && !commandline)	
	    {
	      while (!outputnow)
		{
		  /* Get the value for the number of samples processed */
		  oldvalue = newvalue;

#                 ifdef SunOS
		      ioctl(fd2,AUDIO_GETINFO,&aud);
		      newvalue = aud.play.samples;
#                 endif
#                 ifdef Linux
		      ioctl(fd, SNDCTL_DSP_GETOPTR, &info);
		      newvalue = info.bytes / samplesize;
#                 endif
		  
		  /* Output if no samples have been processed - ie it's all over! */
		      /*		  if (newvalue == 0) 
		    {
		      newvalue = oldvalue;
		      outputnow = 1;
		      ((SharedVUVariables *)VUdata)->sharedexitnow = 1;
		      }*/
		  if (newvalue != oldvalue) 
		    {
		      /* Update the tally */
		      optally = optally + (newvalue - oldvalue);
		      
		      /* Output a value if the tally has breached the required value */
		      if (optally >= rounded_samplerepeat)
			{			
			  outputnow= 1;
			  /* Set the buffer index number to correspond to the required sample */
			  i = i + (optally * samplesize);
			  /* Reset the tally */
			  optally = 0;		
			} else {
			  usleep(2000);
			}
		    }
		}

	      while (outputnow)
		{
		  if (!((SharedVUVariables *)VUdata)->sharedexitnow)
		    {
		      /* Read and process vaules up to byte i */
		      double largest[global_channels];
		      for (chancnt = 0; chancnt < global_channels; chancnt++) 
			largest[chancnt] = 0.0;
		      while (bytesprocessed < i) {

			/* Read in the next buffer if the required
			/* sample is not in the current buffer */
			while (lastbyte == bytesprocessed) {
			  bufstart = lastbyte;
			  bytesread = read(ffd2,buffer,512);
			  lastbyte += bytesread;
			}
			/* If the required sample is in the current buffer, then read it in   */
			/* This way of doing this, is a bit kack-handed...feel free to change */
			/* it, as required						      */
			if (bytesread)
			  {
			    for (chancnt = 0; chancnt < global_channels; chancnt++)
			      {
				for (bitcnt = 0; bitcnt < (global_bits/8); bitcnt++)
				  bitvalue[bitcnt] = buffer[bytesprocessed - bufstart
							   + bitcnt + chancnt*(global_bits/8)];
				switch (global_bits) {
				  case 8:
				    op8 = bitvalue[0];
				    if (op8 < 0) op8 += 256;
				    op8 -= 128;
				    if (op8 < 0) op8 = (~op8) - 1; 
				    if (op8 == 0) op8 = 1;
				    value[chancnt] = (1.0/127.0) * op8;
				    break;

				  case 16:
#                                   ifdef BigEndian
				      op16 = (bitvalue[0] << 8) + bitvalue[1];
#                                   else
				      op16 = (bitvalue[1] << 8) + bitvalue[0];
#                                   endif
				    if (op16 < 0) op16 = (~op16) - 1;
				    if (op16 == 0) op16 = 1;
				    value[chancnt] = (1.0/32767.0) * op16;
				    break;

				  case 32:
#                                   ifdef BigEndian
				      op32 = (bitvalue[0] << 24) + (bitvalue[1] << 16) + (bitvalue[2] << 8) + bitvalue[3];
#                                   else
				      op32 = (bitvalue[3] << 24) + (bitvalue[2] << 16) + (bitvalue[1] << 8) + bitvalue[0];
#                                   endif
				    if (op32 < 0) op32 = (~op32) - 1;
				    if (op32 == 0) op32 = 1;
				    value[chancnt] = (1.0/2147483647.0) * op32;
				    break;
				}
				if (value[chancnt] > largest[chancnt])
				  largest[chancnt] = value[chancnt];
			     }
			    bytesprocessed += samplesize;
			  }
		      }
		      /* Convert rectified value to dB and output */		     			
		      for (opcnt = 0; opcnt < global_channels; opcnt++)
			{
			  valuedBfloat[opcnt] = 20 * log10(largest[opcnt]);
			  valuedB[opcnt] = (int) rint(valuedBfloat[opcnt]);
			  if (valuedB[opcnt] < ((SharedVUVariables*)VUdata)->shareddecay[opcnt]
			      && ((SharedVUVariables *)VUdata)->sharednewvalue)
			    {
			      valuedB[opcnt] = ((SharedVUVariables *)VUdata)->shareddecay[opcnt];	
			    } 
			  ((SharedVUVariables *)VUdata)->sharedvaluedB[opcnt] = valuedB[opcnt];
			  ((SharedVUVariables *)VUdata)->sharedmeterstarted = 1;	     					
			  ((SharedVUVariables *)VUdata)->sharednewvalue = 1;
/*			  if (opcnt == 0) printf("v.%d ",valuedB[opcnt]);
			  else printf("%d ",valuedB[opcnt]);*/
			}		     				
/*		      printf("\n");		     	*/
		      outputnow = 0;
		    }
		}
	      usleep(10000);
	    }
	  while (!((SharedVUVariables *)VUdata)->sharedfinaldecaydone)
	      sleep(1);
	  if (commandline == 1)
	      while (!((SharedVUVariables *)VUdata)->sharedexitnow)
		  sleep(1);
	  kill(pid2,SIGTERM);
	}	
      else
	{
	  /* Do the third fork - the line data */
	  if (pid3 = fork())
	    {
	      signal(SIGTERM, sigquit3);
	      while(line && !((SharedVUVariables *)VUdata)->sharedexitnow && !commandline) 
		{
		  /* Read in the number of samples processed */
#                 ifdef SunOS
		      err = ioctl (fd2,AUDIO_GETINFO,&aud);
		      samplesread = aud.play.samples;
#                 endif
#                 ifdef Linux
		      err = ioctl (fd,SNDCTL_DSP_GETOPTR, &info);
		      samplesread = info.bytes / samplesize;
#                 endif
		  if (samples1 != samplesread ) 
		    {	               	             
		      samplesold = samples1;
		      
#                 ifdef SunOS
		      samples1 = aud.play.samples;
#                 endif
#                 ifdef Linux
		      samples1 = info.bytes / samplesize;
#                 endif
		      
		      if (samples1 == 0) samples1 = samplesold;
		      samples2 = samples1 - samplesold;
		      
		      /* Update the tally */
		      sampletally = sampletally + samples2;
		      
		      /* Output data if the tally has breached the required value */
		      if (sampletally >= roundedsample)
			{
			  /* Increment pixeltally */
			  pixeltally++;
			  
			  /* Work out an overload factor - ie if the number of samples processed */
			  /* since last time is greater than twice the breach-value              */
			  overload = trunc (sampletally,roundedsample) - 1;
			  
			  /* Update the sample tally */
			  sampletally = sampletally - (overload * roundedsample);
			  cntr = cntr + overload;
			  
			  /* Update the fractional-division-error correction tally */
			  if (overload == 0) correctiontally = correctiontally + correctionfactor;
			  else 
			    {
			      /* Update the fractional-division-error correction tally */
			      correctiontally =  (correctionfactor * overload) + correctiontally;
			      
			      /* Print out overload correction */
			      printf("l.%d\n",overload);				
			    }
			  
			  /* Output a value if the fractional-division-error correction tally */
			  /* has become significant - ie it has breached a certain value      */
			  if (correctiontally >= (float) roundedsample)
			    {	                         
			      printf("l.-1\n");
			      correctiontally = correctiontally - roundedsample;
			    }
			  
			  if ((correctiontally * -1) >= (float) roundedsample)
			    {
			      printf("l.1\n");
			      cntr++;
			      correctiontally = correctiontally + roundedsample;
			    }
			  
			  /* Output if the line needs to be advanced by "repeat" number */
			  /* of pixels						      */
			  if (pixeltally == repeat)
			    {
			      /* print out number of pixels to move */
			      printf("l.%d\n",repeat);
			      pixeltally = 0;
			    }
			  cntr++;
			  
			  /* Update sample tally */
			  sampletally = sampletally - roundedsample;
			}	        
		      usleep(50000);
		    }
		}
	      if (commandline == 1)
		  while (!((SharedVUVariables *)VUdata)->sharedexitnow)
		      sleep(1);
	      kill(pid3,SIGTERM);
	    }
	  /* Do the last fork - acually playing the sound-file */
	  else
	    {		
	      while ((l=read(ffd,buffer,next_count))>0)
		  if (write(fd,buffer,l)==-1)
		    {
		      fprintf(stderr,"Can't Write to Card");
		      exit(-1);
		    }
#             ifdef SunOS
		  /* stops soundfile ending prematurely     */
		  /* by continuously checking to see if the */
		  /* device is active before proceeding to  */
		  /* the file "close" statement             */
		  soundcard_active = 1;
		  while (soundcard_active)                                
		    {
                      sleep(1); 
		      err = ioctl (fd,AUDIO_GETINFO,&aud1);
		      if (err == -1) soundcard_active = 0;
		      else soundcard_active = aud1.play.active;
		    }
#	      endif
	      ((SharedVUVariables *)VUdata)->sharedexitnow = 1;
	      close (fd);
	      exit(0);						
	    }	
	}
    }
  /* Close the file, and release the shared memory block */		
  close (ffd);
  shmdt(VUdata);	
  return(0);
}


/*void diagnostics (void)*/
/*checks for bit rate capability	 	*/
/*	     sampling rate range 		*/
/*	     mono/stereo capability	 	*/
/*		mixing facilities 		*/
/* writes to stdout in following format:	*/
/* { BR1 BR2 BR3 } { CH1 CH2 } { CH1_SR_MIN CH1_SR_MAX } { CH2_SR_MIN CH2_SR_MAX} */
/* {valid bitrates} {channels} { sample rate ranges for channels		*/

void diagnostics (void)
{
	
	int i;
	int stflag=0;
	ST_CONFIG orig,parm;
	int *new;
		
	if (!reccard()) 
	{
		fprintf(stderr,"open failed");
		exit(-2);
	}
	
	cardctl(fd,orig,ST_GET); 	/*store original settings, in orig */
	equate_ST_CONFIG(parm,orig);

	printf("\n{ ");
	/*check for bit capability */
	
	for (i=8;i<=MAX_BIT_RATE;i+=8)
	{	
		parm[0]=i;
		cardctl(fd,parm,ST_SET);
		if (parm[0]==i) printf("%d ",i);
		
	}
	
	printf(" } { ");

	/*check for number of channels*/
	for (i=1;i<=MAX_CHANNELS;i++)
	{			
		parm[1]=i;
	        cardctl(fd,parm,ST_SET);
		if (parm[1]==i)  printf("%d ",parm[1]);
		if (parm[1]==2) stflag=1;
	}
	
	printf("} { ");

	/*check for sample rate range */
		
	/*  mono */
	/*minimum rate*/
	
	parm[1]=1;
	parm[2]=MIN_SAMP_RATE;		/*Set Rate to minimum */ 
	cardctl(fd,parm,ST_SET);
	printf("%d ",parm[2]);
	
	/*maximum rate*/
	parm[2]=MAX_SAMP_RATE;	/*Set Rate to highest known sampling rate */ 
	cardctl(fd,parm,ST_SET);
	printf("%d ",parm[2]);

	
	/*stereo (if flag is set)*/
	printf(" } { ");	
	if (stflag) 
	{	
		/*minimum rate*/
		parm[1]=2;
		parm[2]=MIN_SAMP_RATE;		/*Set Rate to minimum */ 
		cardctl(fd,parm,ST_SET);
		printf("%d ",parm[2]);
	
		/*maximum rate*/
		parm[2]=MAX_SAMP_RATE;	/*Set Rate to highest known sampling rate */ 
		cardctl(fd,parm,ST_SET);
		printf("%d ",parm[2]);
	} 
	
	else printf ("0 0");
	

	printf(" }\n");

	/*ioctl(fd,SOUND_PCM_RESET,0);
	ioctl(fd,SOUND_PCM_WRITE_BITS,&parm);
	printf("Current setting: %d",parm);
	ioctl(fd,SOUND_PCM_WRITE_CHANNELS,&parm);
	printf(" %d ",parm);
	ioctl(fd,SOUND_PCM_WRITE_RATE,&parm);
	printf("%d\n",parm);*/
	
	cardctl(fd,orig,1);
	
	return;
}

int reccard (void)
{
	if ((fd=open(DSP_NAME,O_RDONLY,0))==-1) 
	{
		fprintf(stderr,"s_tool.c[reccard]: open failed");
		return(0);
	}
	 
	return (1);
}	

int setcard (void)
{
	if ((fd=open(DSP_NAME,O_WRONLY,0))==-1) 
	{
		fprintf(stderr,"s_tool.c[setcard]: open failed");
		return(0);
	}
	 
	return (1);	
}

/* int *cardctl(int fd,ST_CONFIG parm, int set)				*/
/*cardctl reads the current card configuration if st_flag =ST_GET	*/
/*	  sets   the card configuration	       if st_flag= ST_SET       */
/*	information is passed in integer array of 3			*/
/*	parm[1]= Bit Resolution, 					*/
/*	parm[2]= channels(1 or 2), 					*/
/*	parm[3]= Sampling Rate.						*/

/* RETURNS a an array containing the new seetings of the card		*/
/* NOTE :the card must already be open for reading or writing!		*/
	
int *cardctl (int fd,ST_CONFIG parm,int st_flag)
{	
	static ST_CONFIG temp;
	int i,error,err;
#       ifdef SunOS
        audio_info_t template,aud;
#       endif
        
#       ifdef SunOS        
        	AUDIO_INITINFO (&template);
            	template.play.encoding = (int)AUDIO_ENCODING_LINEAR;
            	template.record.encoding = (int)AUDIO_ENCODING_LINEAR;
            	template.record.buffer_size = 8;
            	err = ioctl (fd,AUDIO_SETINFO, &template);
#       endif
        
	if (st_flag) 
	{

		/*printf ("\nbefore %d -",parm[0]);
		printf ("%d -",parm[1]);
		printf ("%d -\n",parm[2]);*/

#               ifdef Linux
			 if (ioctl(fd,SOUND_PCM_WRITE_BITS,&parm[0])==-1) {puts ("BIt fail"); }	
			 if (ioctl(fd,SOUND_PCM_WRITE_CHANNELS,&parm[1])==-1) {puts ("ch fail");}
			 if (ioctl(fd,SOUND_PCM_WRITE_RATE,&parm[2])==-1) {puts ("rate fail");}
#               endif
#               ifdef SunOS
                	template.play.precision = parm[0];
                 	template.record.precision = parm[0];
                
                 	if (ioctl (fd,AUDIO_SETINFO, &template)==-1) {puts ("BIt fail");}
                
                 	template.play.channels = parm[1];
                 	template.record.channels = parm[1];
                
                	if (ioctl (fd,AUDIO_SETINFO, &template)==-1){puts ("ch fail");}
                
                 	template.play.sample_rate = parm[2];
                 	template.play.sample_rate = parm[2];
                
                 	if (ioctl (fd,AUDIO_SETINFO, &template)==-1) {puts ("rate fail");}
#               endif
	}
	 
	else
        {
#	        ifdef SunOS
               		error = ioctl (fd,AUDIO_GETINFO, &aud);	
	       		parm[0] = aud.play.precision;
	       		parm[1] = aud.play.channels;
	       		parm[2] = aud.play.sample_rate;
#               endif
#               ifdef Linux	    	
	      		error=ioctl(fd,SOUND_PCM_READ_BITS,&parm[0]);
               		/*printf("err:%d\n",error);*/
               		ioctl(fd,SOUND_PCM_READ_CHANNELS,&parm[1]);
               		ioctl(fd,SOUND_PCM_READ_RATE,&parm[2]);
              		 /*equate_ST_CONFIG( parm,temp);*/
#               endif
	}
	return(temp);
}

/*equates arrays a=b */
void equate_ST_CONFIG (ST_CONFIG a, ST_CONFIG b)
{
	int i;
	for (i=0;i<3;i++) a[i]=b[i];
}

void sigquit ()
{
	kill (pid, SIGTERM);
	
  	shmdt(VUdata);
	exit (0);
}

void sigquit3 ()
{
	kill (pid3, SIGTERM);
	
  	shmdt(VUdata);
	exit (0);
}

int round (int i, int j)
{
	int rounded;
    	div_t stuff;
    
   	stuff = div(i,j);
    
   	if ((stuff.rem * 2) >= j) rounded = stuff.quot + 1;
    	else rounded = stuff.quot;
    
	return (rounded);
}

int trunc (int i, int j)
{
	div_t stuff;
    
    	stuff = div(i,j);
    	return (stuff.quot);
}

void sigquit2()
{	
	int chancnt;
	int decay[global_channels];
	int exitnow = 0;
	
	kill (pid2, SIGTERM);
	
  	if (commandline == 0) Final_Decay();
  
  	shmdt(VUdata);
	exit(0);
}

void Final_Decay (void)
{
	int chancnt;
	int exitnow = 0;
	int decay[global_channels];
	long time;
	
	time = global_decay * 10000L;

	
	for (chancnt = 0;chancnt < global_channels;chancnt++)
  	{
  		decay[chancnt] = ((SharedVUVariables *)VUdata)->shareddecay[chancnt];
  	}
	
	while (!exitnow)
  	{
  		Decay_It (decay, 1);
  		
  		for (chancnt = 0;chancnt < global_channels;chancnt++)
  		{
  			if (decay[chancnt] < -50) exitnow = 1;
  			if (decay[chancnt] >= -50) exitnow = 0;	
  		}
  		if (exitnow) ((SharedVUVariables *)VUdata)->sharedfinaldecaydone = 1;
  		usleep(time);
  		
  	}
}

void Decay_It (int decay[global_channels], int final)
{
	int C_chancnt;

	printf("v.");
  	for (C_chancnt = 0;C_chancnt < global_channels;C_chancnt++)
  	{
		printf("%d ", decay[C_chancnt]);
  		if (decay[C_chancnt] >= -2) decay[C_chancnt] = decay[C_chancnt] - 1;
  		else if (decay[C_chancnt] <= -3 && decay[C_chancnt] >= -5) decay[C_chancnt] = decay[C_chancnt] - 2;
  		else if (decay[C_chancnt] <= -6 && decay[C_chancnt] >= -15) decay[C_chancnt] = decay[C_chancnt] - 5;
  		else if (decay[C_chancnt] <= -16 && decay[C_chancnt] >= -35) decay[C_chancnt] = decay[C_chancnt] - 10;
  		else if (decay[C_chancnt] <= -36 && decay[C_chancnt] >= -50) decay[C_chancnt] = decay[C_chancnt] - 15;
  		if (final == 0) ((SharedVUVariables *)VUdata)->shareddecay[C_chancnt] = decay[C_chancnt];
 	}
  	printf("\n");
}
