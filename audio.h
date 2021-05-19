
struct _modulator;
typedef struct _modulator modulator;

typedef void (* filter)(float * out, int sample, modulator * thing);

struct _modulator{
  void * data;
  filter f;
  modulator * sub;
};

typedef struct{
  float frequency;
}sinething;

typedef struct{
  float a,d,s,r;
}asdrthing;

typedef struct {
  float * frequency;
  int * song;
  int count;
  float speed;
  float offset;
}sequencerthing;

typedef struct {
  modulator ** subs;
  int count;
}combinerthing;

typedef struct{
  int samplerate;
}modulator_ctx;

typedef struct {
  modulator * sub1;
  modulator * sub2;
  float gain;
}mixer;

typedef struct {
  
}noisor;

modulator * create_sine(float freq);
modulator * create_envelope(float a, float d, float r, float s);
modulator * create_adsr(float a, float d, float s, float r);
modulator * create_combiner(modulator ** things, int count);
modulator * create_sequencer(int * song, int count, float speed, float * freq, modulator * sub);
modulator * create_mixer(modulator * sub1, modulator * sub2, float gain);
modulator * create_noisor(void);
typedef struct  {
  size_t sample_id;
}audio_sample;

typedef struct {
  size_t stream_id;
}audio_stream;

struct audio_context;
typedef struct _audio_context audio_context;

audio_context * audio_initialize(int sample_rate);
int audio_get_sample_rate(audio_context * ctx);
void audio_context_make_current(audio_context * ctx);
void audio_update_streams(audio_context * ctx);
audio_stream audio_play_stream(audio_context * ctx, void (* gen)(int sample_rate, size_t start_sample, float * buffer, size_t count, void * userdata), void * userdata);
void audio_stop_stream(audio_context * ctx, audio_stream stream);
void audio_remove_stream(audio_context * ctx, audio_stream stream);
audio_sample audio_load_samplef(audio_context * ctx, float * samples, int sample_count);
void audio_play_sample(audio_context * ctx, audio_sample sample);
void audio_set_volume(audio_context * ctx, float value);


float note_to_frequency(int note);

audio_sample audio_load_samplef2(audio_context * ctx, int length, modulator * mod);
