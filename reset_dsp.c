/* RESET_DSP - Reset DSP card */

#include <unistd.h>  // for 'close'
#include <stdlib.h>  // for 'exit'
#include <sys/ioctl.h> // for 'ioctl'

#include <stdio.h>
#include <fcntl.h> // for O_RDONLY, O_WRONLY, etc.
#include <linux/soundcard.h>

int
main()
{
  int dspfd;

  if ((dspfd = open("/dev/dsp", O_WRONLY)) == -1) {
    fprintf(stderr, "Couldn't access sound device\n");
    exit(1);
  }
  else {
    ioctl(dspfd, SNDCTL_DSP_RESET, 0);
    close(dspfd);
  }
  exit(0);
}
