#pragma once
#include <inttypes.h>
#include <iostream>
#include "sound_defs.h"

class Sound;

class OpenAL_Output {
public:
	OpenAL_Output(Sound &SndRef);
	~OpenAL_Output();

	void update_buffer();
	
	unsigned queueSize();

	sample_t *sample_queue;
	unsigned queue_head;
	unsigned queue_tail;

  unsigned queued_buffers;

	Sound &SND;
	ALuint src;

	unsigned queue_capacity;
	unsigned buffer_size;

	unsigned long samples;

  friend std::ostream & operator << (std::ostream & out, const OpenAL_Output & oa);
  friend std::istream & operator >> (std::istream & in, OpenAL_Output & oa);
};