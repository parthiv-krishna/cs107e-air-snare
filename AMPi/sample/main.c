#include <ampi.h>
#include <ampienv.h>

#include <linux/coroutine.h>
#include "common.h"
#include "printf.h"
#include "timer.h"

// Implemented in synth.c or synth_ogg.c
unsigned synth(int16_t **buf, unsigned chunk_size);

int main (void)
{
	printf("Starting...\n");
	timer_delay(1);
	// Start the timer in the environment
	env_init();
	printf("Finished initializing env\n");

	// Initialize audio
	DSB();
	AMPiInitialize(44100, 4000);
	AMPiSetChunkCallback(synth);
	DMB();

	printf("Finished initializing audio\n");

	// Start playback
	while (1) {
		DSB();
		AMPiStart();
		while (AMPiIsActive()) { }
		DMB();

		DSB();
		MsDelay(2000);
		DMB();
	}
}
