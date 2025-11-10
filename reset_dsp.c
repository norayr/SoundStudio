/* RESET_DSP - Reset DSP card */

#include <stdio.h>
#include <stdlib.h>        /* for exit */
#include <fcntl.h>
#include <unistd.h>        /* for close */
#include <sys/ioctl.h>     /* for ioctl */
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
