#pragma once
#include <inttypes.h>
#include <atomic>
#include <thread>
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
	std::thread worker;

	Sound &SND;
	ALuint src;
	ALuint *al_buffers;
	sample_t *zero_buffer;

	std::atomic_flag buffer_lock;
	unsigned buffer_size;
	void audio_worker();
};