#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <limits.h>

#include "types.h"
#include "utils.h"
#include "mem.h"
#include "math.h"
#include "time.h"
#include "log.h"

#include "audio.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>

typedef struct {
  u32 source;
  u32 frontbuffer;
  u32 middlebuffer;
  u32 backbuffer;
  size_t last_sample;
  void (* gen)(int sample_rate, size_t start_sample, float * buffer, size_t count, void * userdata);
  void * userdata;
  float * gen_buffer;
  int gen_buffer_count;
  u32 id;
  bool free;
  bool is_stream;
}stream_info;

static inline float ffmodf(float x, float y) {
	return (x - y * (int)(x / y));
}
static inline float saw(float rads, float shape)
{
    rads = ffmodf((float)rads, (float)PI * 2);

    float t = rads / (float)(PI * 2);
    float a = (shape + 1.0f) / 2.0f;

    if (t < a / 2)
		return 2 * t / a;

    if (t > (1 - (a / 2)))
		return (2 * t - 2) / a;

	return (1 - 2 * t) / (1 - a);
}

static inline float square(float rads){
  rads = ffmodf((float)rads, (float)PI * 2);
  if(rads > PI)
    return 1;
  else return -1;
}

int samplerate = 44100;
void sinefilter(float * out, int sample, modulator * thing){

  float * fd = (float *) &thing->data;
  float f = fd[0];
  float phase = (float)(sample / (float) samplerate) * f * 2 * 3.14;
  //float result = sin(phase) * 0.5;
  float result = saw(phase, 0.0);
  //float result = square(phase);
  out[0] = result;
}

modulator * create_sine(float freq){
  
  modulator * thing = calloc(sizeof(modulator), 1);
  float * fd = (float *)&thing->data;
  fd[0] = freq;
  thing->f = sinefilter;
  return thing;
}

void asdrfilter(float * out, int sample, modulator * thing){
  
  float time = (float)((float)sample / (float) samplerate);
  if(thing->sub == NULL) return;
  thing->sub->f(out, sample, thing->sub);


  asdrthing adsr = *((asdrthing *)thing->data);
  float scale = 1.0;
  if(time < adsr.a){
    scale *= time / adsr.a;
  }else if(time < (adsr.a + adsr.d)){
    // nothing;
    time = time - adsr.a;
    scale *= (1 - (time / adsr.d) * 0.2);
  }else if(time < (adsr.a + adsr.d + adsr.s)){
    scale *= 0.8;
  }else if(time < (adsr.a + adsr.d + adsr.s + adsr.r)){
    time = time - adsr.a - adsr.d - adsr.s;
    scale *=0.8 * (1 - time / adsr.r);
  }else{
    scale = 0;
  }
  *out *= scale;
  
}

modulator * create_adsr(float a, float d, float s, float r){
  modulator * thing = calloc(sizeof(modulator), 1);
  asdrthing * adsr = malloc(sizeof(asdrthing));
  adsr->a = a;
  adsr->d = d;
  adsr->s = s;
  adsr->r = r;
  thing->f = asdrfilter;
  thing->data = adsr;
  return thing;
}

void combinerfilter(float * out, int sample, modulator * thing){
  UNUSED(out);
  out[0] = 0;
  combinerthing * combiner = thing->data;
  for(int i = 0; i < combiner->count; i++){
    float o = 0.0;
    UNUSED(o);
    UNUSED(sample);
    combiner->subs[i]->f(&o, sample, combiner->subs[i]);
    out[0] += o;
  }
}

modulator * create_combiner(modulator ** things, int count){
  var combiner = (combinerthing*) calloc(sizeof(combinerthing), 1);
  var thing = (modulator*) calloc(sizeof(modulator), 1);
  combiner->subs = calloc(sizeof(modulator), count);
  
  for(int i = 0; i <count; i++)
    combiner->subs[i] = things[i];
  combiner->count = count;
  
  thing->data = combiner;
  thing->f = combinerfilter;
  thing->sub = NULL;
  return thing;
}
void modulatorfilter(float * out, int sample, modulator * thing){
  mixer * mod = thing->data;
  float a = 0;
  mod->sub1->f(&a, sample, mod->sub1);
  float b = 0;
  mod->sub2->f(&b, sample, mod->sub2);
  *out = a *(1 - mod->gain) + b * mod->gain;
}
modulator * create_mixer(modulator * sub1, modulator * sub2, float gain){
  mixer * mod = alloc0(sizeof(mixer));
  modulator * thing = alloc0(sizeof(modulator));
  mod->gain = gain;
  mod->sub1 = sub1;
  mod->sub2 = sub2;
  thing->data = mod;
  thing->f = modulatorfilter;
  return thing;
}

static const float kTwoOverUlongMax = 2.0f / (float)ULONG_MAX;

static inline float randf(void)
{
	// Calculate pseudo-random 32 bit number based on linear congruential method.
	// http://www.musicdsp.org/showone.php?id=59
	static unsigned long random = 22222;
	random = (random * 196314165) + 907633515;
	return (float)random * kTwoOverUlongMax - 1.0f;
}

void noisefilter(float * out, int sample, modulator * thing){
  UNUSED(thing);
  UNUSED(sample);
  *out = randf();

}

modulator * create_noisor(){
  modulator * thing = alloc0(sizeof(modulator));
  thing->f = noisefilter;
  return thing;
}
/*
modulator * create_lowpass(modulator * sub, float amount){
  
}*/
static float note_frequency[512];
static bool freq_init = false;
static int low = 12 * 8;
float _note_to_frequency(int note){
  return 440.0 * powf(2.0, ((float)note) / 12);
}

float note_to_frequency(int note){
  if(freq_init == 0){
    freq_init = true;
    for(int i = 0; i < (int)array_count(note_frequency); i++){
      note_frequency[i] = _note_to_frequency(i - low);
    }
  }
  return note_frequency[note + low]; 
}

void sequencerfilter(float * out, int sample, modulator * thing){
  sequencerthing * seq = thing->data;
  float samp = (float)sample / samplerate * seq->speed;
  
  int note_index = ((int)samp);
  int note = seq->song[note_index % seq->count];
  seq->frequency[0] = note_to_frequency(note);
  int newsample = sample - (note_index / seq->speed) * samplerate;
  thing->sub->f(out, newsample, thing->sub);
}

modulator * create_sequencer(int * song, int count, float speed, float * freq, modulator * sub){
  modulator * thing = calloc(sizeof(thing[0]), 1);
  sequencerthing * seq = calloc(sizeof(seq[0]), 1);
  thing->sub = sub;
  thing->data =seq;
  thing->f = sequencerfilter;
  seq->count = count;
  seq->speed = speed;
  seq->frequency = freq;
  seq->song = calloc(sizeof(song[0]), count);
  for(int i = 0; i < count; i++)
    seq->song[i] = song[i];
  return thing;
}

// below is the AL implementation of the audio API.

#include <AL/al.h>
#include <AL/alc.h>


void _error(const char * file, int line, const char * message, ...);
static void check_error(void){
  int err;
  if((err = alGetError()) != AL_NO_ERROR){
    printf("AL ERROR %i\n", err);
#define handle_error(X)    case X: _error(__FILE__,__LINE__, "%s", #X);break;
    switch(err){
      handle_error(AL_INVALID_NAME);
      handle_error(AL_INVALID_ENUM);
      handle_error(AL_INVALID_VALUE);
      handle_error(AL_INVALID_OPERATION);
      handle_error(AL_OUT_OF_MEMORY);
    }
  }
}

static void clear_error(void){
  alGetError();
}

unsigned query_sample_rate_of_audiocontexts(void);

struct _audio_context {
  ALCdevice * al_device;
  ALCcontext * al_context;
  int sample_rate;
  u32 sources_count;
  //
  // two lists of sources are kept one of free sources
  // and one of busy sources. When a source is allocated
  // it is moved to the list of busy sources.
  //
  stream_info sources[16];
  u32 source_busy[16];
  bool broken;
  float volume;
  float target_volume;
  u32 stream_identifier;
};
u32 audio_alloc_source(audio_context * ctx);
stream_info * find_stream(audio_context * ctx, u32 stream_id);

void audio_set_volume(audio_context * ctx, float volume){
  ctx->volume = volume;
  
}

static void list_audio_devices(const ALCchar *devices)
{
        const ALCchar *device = devices, *next = devices + 1;
        size_t len = 0;
	
        fprintf(stdout, "Devices list:\n");
        fprintf(stdout, "----------\n");
        while (device && *device != '\0' && next && *next != '\0') {
                fprintf(stdout, "%s\n", device);
                len = strlen(device);
                device += (len + 1);
                next += (len + 2);
        }
        fprintf(stdout, "----------\n");
}

audio_context * audio_initialize(int sample_rate){
  list_audio_devices(alcGetString(NULL, ALC_DEVICE_SPECIFIER));
  audio_context context = {
    .al_device = alcOpenDevice(NULL),
    .sample_rate = 44100,
    .sources_count = 16,
    .broken = context.al_device == NULL,
    .volume = 1.0,
    .target_volume = 1.0,
    .stream_identifier = 0,
    .al_context = NULL,
    
  };

  if(!context.broken) {
    context.al_context = alcCreateContext(context.al_device, NULL);
    context.sample_rate = sample_rate;//query_sample_rate_of_audiocontexts();
    
    audio_context_make_current(&context);
        
    for(size_t i = 0; i < context.sources_count; i++){
      alGenSources(1, &context.sources[i].source);
      alSourcef(context.sources[i].source, AL_GAIN, 1.0);
      context.sources[i].free = true;
      context.source_busy[i] = 0;
    }
  
    ALfloat listenerPos[] = {0.0, 0.0, 1.0};
    ALfloat listenerVel[] = {0.0, 0.0, 0.0};
    ALfloat listenerOri[] = {0.0, 0.0, -1.0, 0.0, 1.0, 0.0};
    
    alListenerfv(AL_POSITION, listenerPos);
    alListenerfv(AL_VELOCITY, listenerVel);
    alListenerfv(AL_ORIENTATION, listenerOri);
  }
  audio_alloc_source(&context); // reserve index 0 for errors.
  //alListenerf(AL_GAIN, -100.0);
  return IRON_CLONE(context);
}

int audio_get_sample_rate(audio_context * ctx){
  return ctx->sample_rate;
}

bool audio_source_busy(u32 src){
  int state;
  alGetSourcei(src, AL_SOURCE_STATE, &state);
  return state == AL_PLAYING;
}

u32 audio_alloc_source(audio_context * ctx){

  for(u32 i = 0; i < ctx->sources_count; i++){
    if(ctx->sources[i].free){
      ctx->sources[i].free = false;
      return i;
    }
  }
  check_error();
  return 0;
}

u32 audio_alloc_stream(audio_context * ctx){
  u32 busy_src = audio_alloc_source(ctx);
  return busy_src;
}

void audio_dealloc_stream(audio_context * ctx, u32 stream){
  var item = find_stream(ctx, stream);
  if(item != NULL)
    item->id = 0;
}

void audio_context_make_current(audio_context * ctx){
  if(ctx->broken) return;
  alcMakeContextCurrent(ctx->al_context);
  if(ctx->volume != ctx->target_volume){
    printf("setting volume: %f\n", ctx->volume);
    for(size_t i = 0; i < ctx->sources_count; i++){
      alSourcef(ctx->sources[i].source, AL_GAIN, ctx->volume);
    }
    ctx->target_volume = ctx->volume;
  } 
}

audio_sample audio_load_samplef2(audio_context * ctx, int length, modulator * mod){
  f32 * samples = alloc0(length * sizeof(samples[0]));
  for(int i = 0; i < length; i++){
    mod->f(samples + i, i, mod);
  }
  audio_sample smp = audio_load_samplef(ctx, samples, length);
  dealloc(samples);
  return smp;
  
}

audio_sample audio_load_samplef(audio_context * ctx, float * samples, int count){
  ASSERT(!ctx->broken);
  if(ctx->broken) return (audio_sample){0};
  ALuint buffers[1];
  alGenBuffers(1, buffers);

  ALenum format = AL_FORMAT_MONO16;
  short * data = malloc(count * sizeof(short));
  for(int i = 0; i < count; i++)
    data[i] = samples[i] * 30000.0;
  alBufferData(buffers[0], format, data, count, ctx->sample_rate);
  free(data);
  check_error();
  return (audio_sample){.sample_id = buffers[0]};
}

void audio_play_sample(audio_context * ctx, audio_sample sample){
  if(ctx->broken) return;
  ALuint buffer = sample.sample_id;
  u32 src2 = audio_alloc_source(ctx);
  u32 src = ctx->sources[src2].source;
  logd("Source: %i %i\n", src2, src);
  alSourcei(src, AL_BUFFER, buffer);
  //alSourceRewindv(1, &src);
  alSourcePlay(src);
  if(!audio_source_busy(src)){
    printf("AUDIO SOURCE NOT WORKING\n");
  }
  check_error();
}

void audio_stream_fill_buffer(audio_context * ctx, stream_info * info, u32 buffer, size_t sample){
  if(ctx->broken) return;
  info->gen(ctx->sample_rate, sample, info->gen_buffer, info->gen_buffer_count, info->userdata);

  int count = info->gen_buffer_count;
  ALenum format = AL_FORMAT_MONO16;
  short * data = malloc(count * sizeof(short));
  for(int i = 0; i < count; i++)
    data[i] = info->gen_buffer[i] * 20000.0;
  check_error();
  alBufferData(buffer, format, data, count * 2, ctx->sample_rate);
  check_error();
  free(data);
}
void stream_load(audio_context * ctx, stream_info * info){
  check_error();
  audio_stream_fill_buffer(ctx, info, info->frontbuffer, 0);
  info->last_sample += info->gen_buffer_count;
  audio_stream_fill_buffer(ctx, info, info->middlebuffer, info->last_sample);
  info->last_sample += info->gen_buffer_count;
  audio_stream_fill_buffer(ctx, info, info->backbuffer, info->last_sample);
  info->last_sample += info->gen_buffer_count;
  check_error();
  alSourceQueueBuffers(info->source, 1, &info->frontbuffer);
  //printf("buffer: %i\n", info->frontbuffer);
  check_error();
  alSourceQueueBuffers(info->source, 1, &info->middlebuffer);
  check_error();
  alSourceQueueBuffers(info->source, 1, &info->backbuffer);
  alSourcePlay(info->source);
  check_error();
  //printf("loading stream")
}

stream_info * find_stream(audio_context * ctx, u32 stream_id){
  for(u32 i = 0; i < ctx->sources_count; i++){
    if(ctx->sources[i].id == stream_id)
      return &ctx->sources[i];
  }

  return NULL;
}

audio_stream audio_play_stream(audio_context * ctx, void (* gen)(int sample_rate, size_t start_sample, float * buffer, size_t count, void * userdata), void * userdata){
  if(ctx->broken) return (audio_stream){0};
  u32 stream_id = audio_alloc_stream(ctx);
  
  stream_info * info = find_stream(ctx, stream_id);
  ALuint buffers[3];
  alGenBuffers(array_count(buffers), buffers);
  info->frontbuffer = buffers[0];
  info->middlebuffer = buffers[1];
  info->backbuffer = buffers[2];
  info->gen_buffer_count = 512 * 4;
  info->gen_buffer = NULL;
  info->gen_buffer = realloc(info->gen_buffer, sizeof(float) * info->gen_buffer_count);
  info->userdata = userdata;
  info->gen = gen;
  info->last_sample = 0;
  stream_load(ctx, info);
  alSourcePlay(info->source);
  check_error();
  return (audio_stream){.stream_id = info->source};
}

void audio_remove_stream(audio_context * ctx, audio_stream stream){
  if(ctx->broken) return;
  u32 stream_id = stream.stream_id;
  
  stream_info * info = NULL;
  for(u32 i = 0; i < ctx->sources_count;i++)
    if(ctx->sources[i].id == stream_id){
      info = &ctx->sources[i];
      break;
    }
  ASSERT(info != NULL);
  free(info->gen_buffer);
  alSourceStop(info->source);
  u32 buffers[3];
  alSourceUnqueueBuffers(info->source, 3, buffers);
  check_error();
  u32 buf[] = {info->frontbuffer, info->middlebuffer, info->backbuffer};
  alDeleteBuffers(3, buf);
  check_error();
  info->id = 0;
}

bool stream_info_stopped(stream_info * stream){
  int state;
  alGetSourcei(stream->source, AL_SOURCE_STATE, &state);
  return state == AL_STOPPED;
}

void audio_update_streams(audio_context * ctx){
  if(ctx->broken) return;
  if(ctx->volume <= 0.0001) return;
  for(int j = 0; j < 2; j++){
  for(size_t i = 0; i < ctx->sources_count; i++){
    stream_info * info = &ctx->sources[i];
    if(info->free) continue;
    if(i == 0) continue;
    
    int buffers_processed = 0;
    alGetSourcei(info->source, AL_BUFFERS_PROCESSED, &buffers_processed);
    //printf("Processing... %i\n", buffers_processed);

    bool stopped = stream_info_stopped(info);
    if(stopped){
      int buffers_queued = 0;
      alGetSourcei(info->source, AL_BUFFERS_QUEUED, &buffers_queued);
      u32 fb[buffers_queued];
      alSourceUnqueueBuffers(info->source, buffers_queued, fb);
      clear_error();
      check_error();
      if(info->is_stream){
	stream_load(ctx, info);
	continue;
      }else{
	info->free = true;
      }
    }

    if(buffers_processed > 1){
      //printf("WARNING: Audio buffers out of data!\n");
    }
    
    if(buffers_processed == 0) continue;
    
    u32 _frontbuffer = 0;
    alSourceUnqueueBuffers(info->source, 1, &_frontbuffer);
    if(_frontbuffer != info->frontbuffer){
      printf("WARNING: Unexpected buffer!!\n");
    }
    
    if(false){ // this is not reliable
      int current_buffer;
      
      alGetSourcei(info->source, AL_BUFFER, &current_buffer);
      if(current_buffer != (int)info->middlebuffer){
	printf("WARNING: BACKBUFFER not playing!!\n");
      }
    }
    SWAP(info->frontbuffer, info->middlebuffer);
    SWAP(info->middlebuffer, info->backbuffer);
    audio_stream_fill_buffer(ctx, info, info->backbuffer, info->last_sample);
    info->last_sample += info->gen_buffer_count;
    //printf("Queuing buffer: %i\n", info->backbuffer);
    alSourceQueueBuffers(info->source, 1, &info->backbuffer);
    //printf("Enqueue\n");
    //goto rego;
  }
  check_error();
  }
}
