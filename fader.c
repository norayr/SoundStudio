/*
 * fader.c
 *
 * Apply log-style fade-in and fade-out to the input stream
 *
 * Usage:
 *
 * fader <channels> <sample_type> <fade_up_samples> <fade_down_samples>
 *
 * <sample_type> in sox format, signed or unsigned bytes or words (so
 * legal arguments would be "ub", "sw", "uw" or "sb".
 *
 * History:
 *
 * 19th June 1998: Created. NJB.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int fadeinlen, fadeoutlen;
int channels, sampsize, is_signed;
int head = 0;
int tail = 0;
double *buf, *fadein, *fadeout;

/* Read a sample using the global sample parameters from stdin.
   Return true if there are more to be read */

int read_doubles(double result[])
{
  int   i;
  short wsmp;
  char  bsmp;

  static int sample = 1;

  for (i=0; i<channels; ++i) {
    if (sampsize==1) {
      fread(&bsmp, 1, 1, stdin);
      if (is_signed)
	result[i] = bsmp/128.0;
      else
	result[i] = ((unsigned char)bsmp)/128.0 - 1.0;
    } else {
      fread(&wsmp, 2, 1, stdin);
      if (is_signed)
	result[i] = wsmp/32768.0;
      else 
	result[i] = ((unsigned)wsmp)/32768.0 - 1.0;
    }

    /* Apply the fade-in law */
    
    if (sample <= fadeinlen)
      result[i] *= fadein[sample-1];
  }

  ++sample;
  return !feof(stdin);
}

/* Write a value through the buffer to stdout in the same format as
   the input */

void write_doubles(double samp[])
{
  char wbuf[8]; /* max 4 chans x 16 bit */
  int i;

  if (fadeoutlen) {
    ++head;
    head %= fadeoutlen;
  }

  /* If the buffer has wrapped, write out a sample block */

  if (head == tail) {
    for (i=0; i<channels; ++i) {
      if (sampsize==1) {
	if (is_signed)
	  *wbuf = 127.0 * buf[channels*tail + i];
	else 
	  *((unsigned char *)wbuf) = 128.0 + 127.0 * buf[channels*tail + i];
	fwrite(wbuf, 1, 1, stdout);
      } else {
	if (is_signed)
	  *((int *)wbuf) = 32767 * buf[channels*tail + i];
	else 
	  *((unsigned int *)wbuf) = 32768 + 32767 * buf[channels*tail + i];
	fwrite(wbuf, 2, 1, stdout);
      }
    }

    if (fadeoutlen) {
      ++tail;
      tail %= fadeoutlen;
    }
  }

  /* Save the new sample block in the buffer */

  for (i=0; i<channels; ++i)
    buf[head*channels + i] = samp[i];
}

/* Apply the fade-out and push flush what remains in the buffer to
   stdout.  The scaling is applied in situ, then write calls are made
   with zero data until the buffer has been flushed. */

void flush_samples(void)
{
  int i, j;
  double zeros[] = {0, 0, 0, 0};

  if (fadeoutlen) /* Only take action if fadeout is required */
    for (i=0; i<fadeoutlen; ++i)
      for (j=0; j<channels; ++j)
	buf[channels*((tail+i)%fadeoutlen) + j] *= fadeout[i];

  for (i=0; i<fadeoutlen; ++i)
    write_doubles(zeros);
}

/* The main program... */

int main(int argc, char *argv[])
{
  double samp[4];
  int i;

  channels   = atoi(argv[1]);
  fadeinlen  = atoi(argv[3]);
  fadeoutlen = atoi(argv[4]);
  sampsize   = argv[2][1]=='b' ? 1 : 2;
  is_signed  = argv[2][0]=='s';

  /* Allocate buffer for fade-down and zero it */

  if ((buf = (double *)calloc(fadeoutlen * channels, sizeof(double)))
      == NULL) {
    fputs("fader: Can't allocate sample buffer\n", stderr);
    exit(1);
  }

  /* Precompute the gain change so that we don't pose unnecessarily lumpy
  processing demands when streaming.  Allocate memory for lookup tabs */

  if ((fadein = (double *)calloc(fadeinlen, sizeof(double))) == NULL
      || (fadeout = (double *)calloc(fadeoutlen, sizeof(double))) == NULL) {
    fputs("fader: Can't allocate lookup table\n", stderr);
    exit(1);
  }

  /* The law we are using is as follows: linear from 0 to -60db which
  is 10% of the way through the fade-in.  Then exponential
  increase. */

  for (i=0; i<fadeinlen; ++i)
    if (i < fadeinlen/10)
      fadein[i] = 1e-2 * (double)i/fadeinlen;
    else 
      fadein[i] = 0.0004641588 * exp(7.6752836 * (double)i/fadeinlen);

  for (i=0; i<fadeoutlen; ++i)
    if (i < 0.9*fadeoutlen)
      fadeout[i] = 0.0004641588 *
	exp(7.6752836 * (double)(fadeoutlen - i)/fadeoutlen);
    else 
      fadeout[i] = 1e-2 * (double)(fadeoutlen - i)/fadeoutlen;

  /* Now copy the std input to std output */

  while (read_doubles(samp))
    write_doubles(samp);

  /* Perform fadeout and finish */

  flush_samples();
  free(buf);

  exit(0);
}

