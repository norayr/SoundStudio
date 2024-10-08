/* MaxMin                  */
/* A program to return the maxima and minima of blocks in 8 & 16 bit samples   */
/* Designed for use with TCL              */
/* Part of the 3rd Year Project by Paul D.Sharpe        */
/* University of Leeds, Dept. of Electronic & Electrical Engineering     */
/*                    */
/* Programmed by Paul D. Sharpe              */
/* Revision : 3                  */
/* Date : 14/2/95                */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/* Forward Declarations */
int maxminstereo (long blocksize, int filetype, char *filename);

/********************************************************************************/
/* Three arguments are required from the Command line. The first is the size of  */
/* the blocks to be scanned in samples (regardless of wordsize);     */
/* The second argument is an integer in the range 0<=arg2<=3; to mean:    */
/* 0 = 8 bit mono; 1 = 8 bit stereo; 2 = 16 bit mono; 3 = 16 bit stereo;  */
/* The third is the filename of the file to be scanned.        */

/* All errors return -1, with a message to stderr.        */
int main(int argc, char *argv[])
{
  /*Variable Declarations */
  int blcsz,fltyp, kanal;
  char filename[100];
  /* Check that sufficient arguments have been given */
  if (argc>3) {
      /* Convert the argument strings to numbers */
      /*Note:Errors are detected in the maxmin function */
      blcsz=(int)strtol (argv[1],(char**)NULL,10);
      fltyp=(int)strtol(argv[2],(char**)NULL,10);
      strcpy(filename,argv[3]);

      /* Call the function that does all the work */
      maxminstereo (blcsz,fltyp,filename);
  }
  else {
   fprintf(stderr,"MaxMin Error:invalid number of arguments: need blocksize and map\n");
    return (-1);
  }
  return(0);
}
/********************************************************************************/
/* int maxmin (long blocksize, int filetype, char *filename)          */
/* This function scans a file and works out the         */
/* maximum and minimum values of the samples within in blocks of     */
/* size 'blocksize'.                 */
/* The file type is an integer indicating the number of channels and    */
/* wordsize of the sample;                 */
/* 0 = 8 bit mono                */
/* 1 = 8 bit stereo                */
/* 2 = 16 bit mono                */
/* 3 = 16 bit stereo                */
/* ie first bit mono/stereo, second bit 8/16bit         */
/* Note that the blocksize must be even, especially if stereo.      */
/* It also counts the number of samples in the file.        */
/* The results are send to the stdout as a TCL list with two elements;    */
/*   1 : a list of the minima and maxima; { min1 max1 }...{minN maxN}  */
/*   2 : the number of samples in the file { total }        */
/* These can be easily accesed in tcl using lindex.        */
/* The filename is simply the name of the file to be scanned      */

int maxminstereo (long blocksize, int filetype, char *filename)
{

   /*Variable Declarations               */
    /* file descriptor */
  int fd;
  /* buffers to contain the blocks to be scanned */
  unsigned char *buffer8;
  short *buffer16;
  /* pointer to assist in read operation */
  char *readptr;
  /* variables to contain the maximum and minimum values */
        unsigned char min8left, max8left, min8right, max8right, value8left, value8right;
  short min16left, max16left, min16right, max16right, value16left, value16right, test;
  float normaxleft, norminleft, normaxright, norminright;  /* (for the normalised results) */
  /* variable to add up the samples */
  long total=0;
  /* flags and counting variables */
  int i,got,stereo;
  int next_count,bytes;

    /* Open the file                */

    if ((fd=open(filename,(O_RDONLY),0))==-1)
    {
      fprintf(stderr,"maxmin Error:file open failed");
      exit(-1);
    }

   /* Check for stereo bit and convert blocksize into bytes accordingly.  */
   if (1&filetype) {
      stereo=1;
      bytes=blocksize*2;
          }
   else      {
      stereo=0;
      bytes=blocksize;
       }
   /* Print the initial bracket to surround the list        */
   printf("{");

   /* Check that the blocksize is not zero          */
   if (blocksize>0)
      {
  /* Scan the file in blocks.            */
  /* Because the data type for 8 bit and 16 bit samples is different these*/
  /* are kept separate.               */
  /* 8 bit data is of type unsigned char.          */
  /* 16 bit data is of type signed short.          */

  /* The 8 bit case.              */
  /* The second bit of filetype indicates whether 8 or 16 bit, hence the  */
  /* bitwise operation in the condition.          */
  if (!(filetype>>1))
  {
    /* Allocate memory for the buffer.          */
      buffer8=(unsigned char*) calloc (bytes,sizeof(unsigned char));
    /* Check that memory was allocated correctly */
    if (buffer8==NULL)
      {
      fprintf (stderr,"maxmin: Malloc Error");
      exit(-1);
      }

    /* Loop to scan the file; continue until end of file    */
    while ((bytes > 0) && (got=read(fd,buffer8,bytes))>0 )
    {
      /* Check for errors during read        */
      if (got== -1)
        {
           fprintf (stderr,"maxmin: Error in reading file.");
           exit(-1);
        }

      /*Set minima and maxima testers to the central position;*/
      /* recall that the data is unsigned char, hence the 128.*/
      min8left=128;
      max8left=128;
      min8right=128;
      max8right=128;

      /* Loop to scan the block.        */
      for (i=0; i<got; i++)
      {
         /*Average stereo channels -        */
         /*(last sample is not averaged,if blocksize not even)*/
         if ((stereo)&&((2*i+1)<got))
           {
                                             value8left=buffer8[2*i];
                                             value8right=buffer8[2*i+1];

        /* Average */
        /* buffer8[i]=(buffer8[i]+buffer8[i+1] )/2;  */
        /* Increment i to account for the stereo word  */
        /* i++;  */

           }
      /*Compare the current value with the previous max & min  */
      if (value8left>max8left) { max8left=value8left; }
      if (value8left<min8left) { min8left=value8left; }
      if (value8right>max8right) { max8right=value8right; }
      if (value8right<min8right) { min8right=value8right; }

      /* Count the total number of samples      */
      total++;
    }

    /* The max & min values are inverted and normalised here.  */
    /* This is for the purpose of plotting on the tk canvas,   */
    /* The coordinate system requires that the maximum is smaller  */
    /* than the minimum.            */
    /* It is normalised for flexibility.        */

                    if (!(stereo))
                      {
                         max8left=128;
                         min8left=128;
                         max8right=128;
                         min8right=128;
                      }

    normaxleft=(float)(255-max8left)/255;
    norminleft=(float)(255-min8left)/255;
    normaxright=(float)(255-max8right)/255;
    norminright=(float)(255-min8right)/255;

    /* Print the normalised, inverted maxima and minima to stdout   */
    printf("{ %1.6f %1.6f %1.6f %1.6f } ",norminleft,normaxleft,norminright,normaxright);

    /* Check whether the amount of bytes requested were obtained.  */
    /* If not end of file has been reached        */
    if (got < bytes)
      bytes = 0;
      }

    /* free allocated memory           */
    free (buffer8);
  }
  /* 16 bit case.                */
  else
  {

    /* Allocate memory for the buffer. (no.of blocks=bytes)    */
    buffer16=(short*) (calloc (bytes, sizeof(short) ) );

    /* Check that memory was allocated correctly */
    if (buffer16==NULL)
      {
           fprintf (stderr,"maxmin:Malloc Error");
        exit(-1);
      }

    /* convert blocksize further to actual bytes      */
    bytes *=2;

    /* Set a char pointer for read function        */
    readptr=(char *) buffer16;

    /* Loop to scan the file; continue until end of file    */
    while ((bytes > 0) && (got=read(fd,readptr,bytes))>0 )
    {
      /* Check for errors during read        */
      if (got== -1)
        {
           fprintf (stderr,"maxmin: Error in reading file.");
           exit(-1);
        }

      /*Set minima and maxima testers to the central position;*/
      min16left=0;
      max16left=0;
      min16right=0;
      max16right=0;

    /* Loop to scan the block.          */
    for (i=0; i<(got/2) ; i++)
    {
      /*Average stereo channels -        */
      /*(last sample is not averaged,if blocksize not even)  */
      /* if ((stereo)&&((i+1)<(got/2))) */
      if ((stereo)&&((2*i+1)<(got/2)))
        {
                                            value16left=buffer16[2*i];
                                            value16right=buffer16[2*i+1];

                                       /* Average          */
          /* buffer16[i]=(buffer16[i]+buffer16[i+1] )/2;  */
          /* Increment i to account for the stereo word  */
          /* i++;  */

        }

      /*Compare the current value with the previous max & min  */
      if (value16left>max16left) { max16left=value16left; }
      if (value16left<min16left) { min16left=value16left; }
      if (value16right>max16right) { max16right=value16right; }
      if (value16right<min16right) { min16right=value16right; }

      /*Count the total number of samples*/
      total++;
    }

    /* The max & min values are inverted and normalised here.  */
    /* This is for the purpose of plotting on the tk canvas,   */
    /* The coordinate system requires that the maximum is smaller  */
    /* than the minimum.            */
    /* It is normalised for flexibility.        */

                    if (!(stereo))
                      {
                         max16left=0;
                         min16left=0;
                         max16right=0;
                         min16right=0;
                      }

    normaxleft= (float)(32767-max16left)/65535;
    norminleft= (float)(32767-min16left)/65535;
    normaxright= (float)(32767-max16right)/65535;
    norminright= (float)(32767-min16right)/65535;

    /* Print the normalised, inverted maxima and minima to stdout   */
    printf("{ %1.6f %1.6f %1.6f %1.6f } ",norminleft,normaxleft,norminright,normaxright);

    /* Check whether the amount of bytes requested were obtained.  */
    /* If not end of file has been reached        */
    if (got < bytes)
      bytes = 0;
      }

    /* free allocated memory           */
    free (buffer16);
       }
   }
   else {
     /*If it has come to this, an invalid blocksize has been given  */
     fprintf  (stderr, "maxmin error: invalid blocksize, must be >0.");
     exit (-1);
  }

   close(fd);
   /* Print the closing braces and the total samples to stdout.      */
   printf ("} {{ %ld }}",total);

   /* Exit normally */
   exit (0);
}

