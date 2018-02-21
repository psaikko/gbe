#include "openal_output.h"
#include "sound.h"

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <AL/alc.h>
#include <AL/alut.h>

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
        for(; err != AL_NO_ERROR; err = alGetError()) { \
            std::cerr << "AL Error " << al_err_str(err) << " at " << (file) << ":" << (line) << std::endl; \
        } \
    }while(false)

#define al_check_error() \
    __al_check_error(__FILE__, __LINE__)

void init_al() {
  ALCdevice *dev = nullptr;
  ALCcontext *ctx = nullptr;

  const char *defname = alcGetString(nullptr, ALC_DEFAULT_DEVICE_SPECIFIER);
  std::cout << "Default device: " << defname << std::endl;

  dev = alcOpenDevice(defname);
  ctx = alcCreateContext(dev, nullptr);
  alcMakeContextCurrent(ctx);
}

void exit_al() {
  ALCdevice *dev = nullptr;
  ALCcontext *ctx = nullptr;
  ctx = alcGetCurrentContext();
  dev = alcGetContextsDevice(ctx);

  alcMakeContextCurrent(nullptr);
  alcDestroyContext(ctx);
  alcCloseDevice(dev);
}

OpenAL_Output::OpenAL_Output(Sound &SndRef) : SND(SndRef), queue_head(0), queue_tail(0), samples(0),
  queued_buffers(0) {
	init_al();

  buffer_size = 2 * SAMPLE_RATE * AL_BUFFER_LEN_MS / 1000;

  /* Set-up sound source and play buffer */
  src = 0;
  alGenSources(1, &src);

  queue_capacity = 2 * SAMPLE_RATE * QUEUE_LEN_MS / 1000;
  // add buffer size to queue for easier reads from end
  sample_queue = new sample_t[queue_capacity + buffer_size];
}

OpenAL_Output::~OpenAL_Output() {
	/* Dealloc OpenAL */
	delete sample_queue;
  exit_al();
}

unsigned OpenAL_Output::queueSize() {
  return (queue_capacity + queue_tail - queue_head) % queue_capacity;
}

void OpenAL_Output::update_buffer() {

	if (SND.hasNewSample()) {

    samples++;

		sample_t left, right;
		SND.getSamples(&left, &right);

		assert((queue_tail + 2) % queue_capacity != queue_head);

		sample_queue[queue_tail] = left;
		sample_queue[queue_tail + 1] = right;
    if (queue_tail < buffer_size) {
      sample_queue[buffer_size + queue_tail] = left;
      sample_queue[buffer_size + queue_tail + 1] = right;
    } else {
      //printf("[snd] queue full\n");
    }
		queue_tail = (queue_tail + 2) % queue_capacity;
    

    if (queueSize() >= buffer_size) {

      sample_t *buffer = &sample_queue[queue_head];
      queue_head = (queue_head + buffer_size) % queue_capacity;
      
      ALuint new_buffer;
      alGenBuffers(1, &new_buffer);
      al_check_error();

      alBufferData(new_buffer, FORMAT, buffer, buffer_size * sizeof(sample_t), SAMPLE_RATE);

      ++queued_buffers;
      alSourceQueueBuffers( src, 1, &new_buffer );

      ALint val;
      alGetSourcei(src, AL_SOURCE_STATE, &val);
      if(val != AL_PLAYING) 
        alSourcePlay(src);

      ALint Processed;
      alGetSourcei( src, AL_BUFFERS_PROCESSED, &Processed );

      while (Processed--) {
        ALuint BufID;
        --queued_buffers;
        alSourceUnqueueBuffers( src, 1, &BufID );
        alDeleteBuffers(src, &BufID);
        al_check_error();
      }

      //if (queued_buffers% 10 == 0)
      //  printf("%ld\n", queued_buffers);
    }
	}
}

ostream &operator<<(ostream &out, const OpenAL_Output & oa) {

  out.write(reinterpret_cast<const char*>(oa.sample_queue), sizeof(sample_t) * oa.queue_capacity);
  out.write(reinterpret_cast<const char*>(&oa.queue_head), sizeof(unsigned));
  out.write(reinterpret_cast<const char*>(&oa.queue_tail), sizeof(unsigned));

  //out.write(reinterpret_cast<const char*>(&oa.queued_buffers), sizeof(unsigned));
  return out;
}

istream &operator>>(istream &in, OpenAL_Output & oa) {

  in.read(reinterpret_cast<char*>(oa.sample_queue), sizeof(sample_t) * oa.queue_capacity);
  in.read(reinterpret_cast<char*>(&oa.queue_head), sizeof(unsigned));
  in.read(reinterpret_cast<char*>(&oa.queue_tail), sizeof(unsigned));


  //unsigned old_buffers = oa.queued_buffers;
  //in.read(reinterpret_cast<char*>(&oa.queued_buffers), sizeof(unsigned));

  /*

  ALuint new_buffer;
  alGenBuffers(1, &new_buffer);
  al_check_error();

  sample_t *buffer = &oa.sample_queue[oa.queue_head];
  alBufferData(new_buffer, FORMAT, buffer, oa.buffer_size * sizeof(sample_t), SAMPLE_RATE);

  for (unsigned i = 0; i < oa.queued_buffers - old_buffers; ++i) {
    alSourceQueueBuffers( oa.src, 1, &new_buffer );
  }

  ALint val;
  alGetSourcei(oa.src, AL_SOURCE_STATE, &val);
  if(val != AL_PLAYING)
    alSourcePlay(oa.src);

  */

  return in;
}
