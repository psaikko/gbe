#pragma once
#include <inttypes.h>
#include "sound_defs.h"

class Sound;

class OpenAL_Output {
public:
	OpenAL_Output(Sound &SndRef);
	~OpenAL_Output();

	void update_buffer();
	
	unsigned queueSize();

	sample_t *sample_queue;
	unsigned queue_capacity;
	unsigned queue_head;
	unsigned queue_tail;

	Sound &SND;
	ALuint src;

	unsigned buffer_size;
};