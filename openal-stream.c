// Stream some noise using OpenAL
// Linux: cc openal-stream.c `pkg-config --cflags --libs openal` && ./a.out
// macOS: cc openal-stream.c -framework OpenAL -DMACOS_BUILTIN_OPENAL && ./a.out
// web: emcc openal-stream.c -lopenal -o index.html
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include <math.h>

#ifdef MACOS_BUILTIN_OPENAL
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#else
#include <AL/al.h>
#include <AL/alc.h>
#endif

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#else
#include <unistd.h>
#endif

#define PCM_SIZE 1024
float pcm[PCM_SIZE];
ALuint source = 0;
unsigned counter;

static void stream_buffer(ALuint bufferId) {
  for (unsigned i = 0; i < PCM_SIZE; ++i)
    pcm[i] = (ALfloat)sin((++counter / 2) / 5.0) * .2;
  alBufferData(
    bufferId, 0x10011/* AL_FORMAT_STEREO_FLOAT32 */, pcm,
    sizeof (pcm), 44000/* Your buffer sample rate */
  );
}

static void iter() {
  ALint processed, state;
  alGetSourcei(source, AL_SOURCE_STATE, &state);
  alGetSourcei(source, AL_BUFFERS_PROCESSED, &processed);
  assert(alGetError() == AL_NO_ERROR);

  while (processed > 0) {
    ALuint buffer;
    alSourceUnqueueBuffers(source, 1, &buffer);
    processed--;
    stream_buffer(buffer);
    alSourceQueueBuffers(source, 1, &buffer);
  }

  if (state != AL_PLAYING && state != AL_PAUSED) {
    fprintf(stderr, "Buffer underrun, restart the play\n");
    alSourcePlay(source);
  }
}

int main() {
  ALCdevice *device = alcOpenDevice(0);
  assert(device);
  ALCcontext *ctx = alcCreateContext(device, 0);
  assert(ctx);
  assert(alcMakeContextCurrent(ctx));
  // The only natively supported one in WebAudio
  assert(alIsExtensionPresent("AL_EXT_float32"));

  alGenSources(1, &source);
  {
    enum { BUFFERS_COUNT = 4 };
    ALuint buffers[BUFFERS_COUNT] = {};
    alGenBuffers(BUFFERS_COUNT, buffers);
    for (unsigned i = 0; i < BUFFERS_COUNT; ++i) stream_buffer(buffers[i]);
    alSourceQueueBuffers(source, BUFFERS_COUNT, buffers);
  }
  alSourcePlay(source);

#ifdef __EMSCRIPTEN__
  emscripten_set_main_loop(iter, 0, 0);
#else
  while (1) {
    usleep(16);
    iter();
  }
#endif
  return 0;
}
