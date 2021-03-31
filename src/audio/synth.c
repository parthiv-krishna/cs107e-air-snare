#include <stdint.h>
#include "audio_sequence.h"
#include "printf.h"


unsigned synth(int16_t **o_buf, unsigned chunk_size)
{
	static int16_t buf[8192];
	*o_buf = &buf[0];

	dumpAllTracks(buf, chunk_size);
	return chunk_size;
}
