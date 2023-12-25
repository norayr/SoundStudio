/* A program that prints out the first N bytes of a specified file */
/* This was written to replace "head" on the Solaris version of    */
/* SoundStudio due to the fact that the Solaris head command will  */
/* only work in line numbers, not bytes                            */
  
/* Usage : heady <NUMBER>                                          */

/* Eg % cat text.txt | heady 10  will printout to standard output  */
/*                               the first 10 bytes of the file    */
/*                               test.txt                          */

#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define READ_BLOCK_SIZE 4096

int filehead (int number);

int main(int argc, char *argv[])
{
   filehead (atoi(argv[1]));
   exit(0);
}       
      
int filehead (int number)
{
  char buff[READ_BLOCK_SIZE];
  int i, fptr;

  i = 0;

  while (i < number) {
    int read;
    int to_go = number-i;
    if (to_go > READ_BLOCK_SIZE)
      to_go = READ_BLOCK_SIZE;
    read = fread(buff, 1, to_go, stdin);
    fwrite(buff, 1, read, stdout);
    if (read < to_go) /* At the end of the file... */
      break;
    else
      i += read;
  }
  
  close (fptr);
   
  return 0;
}        
        
	
	  
