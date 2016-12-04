#include "openal_output.h"
#include "sound.h"

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <AL/alc.h>
#include <AL/alut.h>

#include <thread>
#include <chrono>

using namespace std;
using namespace std::literals;

#define CASE_RETURN(err) case (err): return #err
const char* al_err_str(ALenum err) {
    switch(err) {
        CASE_RETURN(AL_NO_ERROR);
        CASE_RETURN(AL_INVALID_NAME);
        CASE_RETURN(AL_INVALID_ENUM);
        CASE_RETURN(AL_INVALID_VALUE);
        CASE_RETURN(AL_INVALID_OPERATION);
        CASE_RETURN(AL_OUT_OF_MEMORY);
    }
    return "unknown";
}
#undef CASE_RETURN

#define __al_check_error(file,line) \
    do { \
        ALenum err = alGetError(); \
        for(; err!=AL_NO_ERROR; err=alGetError()) { \
            std::cerr << "AL Error " << al_err_str(err) << " at " << file << ":" << line << std::endl; \
        } \
    }while(0)

#define al_check_error() \
    __al_check_error(__FILE__, __LINE__)

void init_al() {
  ALCdevice *dev = NULL;
  ALCcontext *ctx = NULL;

  const char *defname = alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER);
  std::cout << "Default device: " << defname << std::endl;

  dev = alcOpenDevice(defname);
  ctx = alcCreateContext(dev, NULL);
  alcMakeContextCurrent(ctx);
}

void exit_al() {
  ALCdevice *dev = NULL;
  ALCcontext *ctx = NULL;
  ctx = alcGetCurrentContext();
  dev = alcGetContextsDevice(ctx);

  alcMakeContextCurrent(NULL);
  alcDestroyContext(ctx);
  alcCloseDevice(dev);
}

OpenAL_Output::OpenAL_Output(Sound &SndRef) : SND(SndRef), queue_head(0), queue_tail(0) {
	init_al();

  /* Create buffer to store samples */
  al_buffers = new ALuint[N_AL_BUFFERS];
  alGenBuffers(N_AL_BUFFERS, al_buffers);
  al_check_error();

  buffer_size = 2 * SAMPLE_RATE * AL_BUFFER_LEN_MS / 1000;

  zero_buffer = new sample_t[buffer_size];
  memset(zero_buffer, 0, buffer_size * sizeof(sample_t));

 	for (unsigned i = 0; i < N_AL_BUFFERS; ++i) {
 		alBufferData(al_buffers[i], FORMAT, zero_buffer, buffer_size, SAMPLE_RATE);
 		al_check_error();
 	}

  /* Set-up sound source and play buffer */
  src = 0;
  alGenSources(1, &src);

  for (unsigned i = 0; i < N_AL_BUFFERS; ++i)
     alSourceQueueBuffers( src, 1, &al_buffers[i] );
  alSourcePlay(src);

  worker = std::thread(&OpenAL_Output::audio_worker, this);

  queue_capacity = SAMPLE_RATE * 2;
  // add buffer size to queue for easier reads from end
  sample_queue = new sample_t[queue_capacity + buffer_size];
}

OpenAL_Output::~OpenAL_Output() {
	/* Dealloc OpenAL */
	delete zero_buffer;
	delete sample_queue;
  exit_al();
}

unsigned OpenAL_Output::queueSize() {
  return (queue_capacity + queue_tail - queue_head) % queue_capacity;
}

void OpenAL_Output::update_buffer() {
	while (buffer_lock.test_and_set())
			;	

	if (SND.hasNewSample()) {
		sample_t left, right;
		SND.getSamples(&left, &right);
		if ((queue_tail + 2) % queue_capacity == queue_head || (queue_tail + 1) % queue_capacity == queue_head ) {
			//printf("[al_output] sample queue full\n");
		} else {
			sample_queue[queue_tail] = left;
			sample_queue[queue_tail + 1] = right;
      if (queue_tail < buffer_size) {
        sample_queue[buffer_size + queue_tail] = left;
        sample_queue[buffer_size + queue_tail + 1] = right;
      }
			queue_tail = (queue_tail + 2) % queue_capacity;
      if (queue_tail % 100 == 0) {
        //printf("[al_output] queue size %u (%u, %u)\n", queueSize(), queue_head, queue_tail);
      }
		}
	}
	buffer_lock.clear();
}

void OpenAL_Output::audio_worker() {
	while(1) {
		ALint Processed;
	  alGetSourcei( src, AL_BUFFERS_PROCESSED, &Processed );
	  al_check_error();
    //printf("[buffer thread] processed %d\n", Processed);
    // Queue another buffer
    if (Processed) {
        ALuint BufID;
        alSourceUnqueueBuffers( src, 1, &BufID );

        while (buffer_lock.test_and_set())
        	; 

        sample_t *buffer;
        //printf("[buffer thread] queue size %u\n", queueSize());
        if (queueSize() >= buffer_size) {
          unsigned old_size = queueSize();
        	buffer = &sample_queue[queue_head];
        	queue_head = (queue_head + buffer_size) % queue_capacity;
        	//printf("[buffer thread] size %u -> %u (%u, %u)\n", old_size, queueSize(), queue_head, queue_tail);
        } else {
        	buffer = zero_buffer;
        	printf("[buffer thread] no data\n");
        }

        alBufferData(BufID, FORMAT, buffer, buffer_size * sizeof(sample_t), SAMPLE_RATE);

				buffer_lock.clear();

        alSourceQueueBuffers( src, 1, &BufID );

        ALint val;
        alGetSourcei(src, AL_SOURCE_STATE, &val);
        if(val != AL_PLAYING) 
        	alSourcePlay(src);
    }
		this_thread::sleep_for(2ms);
	}
}