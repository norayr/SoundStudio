/* File: s_tool.h */

/* How fast is your machine?  If you reduce this number,
   the line will move more smoothly, but more CPU time will
   be used.  It is a log_2 number, subtract 1 to double the
   smoothness, add 1 to halve it. */

#define AUDIO_FRAGSIZE 9


/* Platform dependencies resolved. */

/* 8/Jan/1999: Created.  R Whitehead */

typedef struct {
	volatile int sharedvaluedB[4];
	volatile int shareddecay[4];
	volatile int sharednewvalue;
	volatile int sharedexitnow;
	volatile int sharedfinaldecaydone;
	volatile int sharedmeterstarted;
	} SharedVUVariables;



#define OS_SUNOS 1
#define OS_LINUX 2

#ifdef Linux

#include <sys/soundcard.h>
     
#define DSP_NAME	"/dev/dsp"
#define MIX_NAME	"/dev/mixer" 
#define MIN_SAMP_RATE   0
#define PLATFORM        OS_LINUX
#define MAX_SAMP_RATE   44100

#endif

#ifdef SunOS     

#include </usr/include/sys/ioctl.h>
#include </usr/include/sys/audioio.h>
    
#define DSP_NAME	"/dev/audio"
#define MIN_SAMP_RATE   8000
#define PLATFORM        OS_SUNOS
#define MAX_SAMP_RATE   48000

	typedef struct {
		int bytes;
	} count_info;

#endif    
