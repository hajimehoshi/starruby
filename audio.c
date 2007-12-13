#include "starruby.h"

static bool bgmLoop = false;
static Uint32 bgmPosition = 0;
static int bgmVolume = 255;
static Mix_Music* sdlBgm = NULL;
static Uint32 sdlPreviousTicks = 0;

static VALUE symbol_loop;
static VALUE symbol_panning;
static VALUE symbol_position;
static VALUE symbol_time;
static VALUE symbol_volume;

static VALUE Audio_bgm_position(VALUE self)
{
  return LONG2NUM(bgmPosition);
}

static VALUE Audio_bgm_volume(VALUE self)
{
  return INT2NUM(bgmVolume);
}

static VALUE Audio_bgm_volume_eq(VALUE self, VALUE rbVolume)
{
  bgmVolume = NORMALIZE(NUM2INT(rbVolume), 0, 255);
  Mix_VolumeMusic(DIV255((int)(bgmVolume * MIX_MAX_VOLUME)));
  return INT2NUM(bgmVolume);
}

static VALUE Audio_play_bgm(int argc, VALUE* argv, VALUE self)
{
  VALUE rbPath, rbOptions;
  rb_scan_args(argc, argv, "11", &rbPath, &rbOptions);
  if (NIL_P(rbOptions))
    rbOptions = rb_hash_new();

  VALUE rbCompletePath = GetCompletePath(rbPath, true);
  char* path = StringValuePtr(rbCompletePath);
  sdlBgm = Mix_LoadMUS(path);
  if (!sdlBgm)
    rb_raise_sdl_mix_error();

  int time   = 0;
  int volume = 256;

  Check_Type(rbOptions, T_HASH);
  VALUE val;
  bgmLoop = RTEST(rb_hash_aref(rbOptions, symbol_loop));
  if (!NIL_P(val = rb_hash_aref(rbOptions, symbol_position)))
    bgmPosition = MAX(NUM2INT(val), 0);
  if (!NIL_P(val = rb_hash_aref(rbOptions, symbol_time)))
    time = NUM2INT(val);
  if (!NIL_P(val = rb_hash_aref(rbOptions, symbol_volume)))
    volume = NORMALIZE(NUM2INT(val), 0, 255);

  Audio_bgm_volume_eq(self, INT2NUM(volume));
  if (time <= 50)
    Mix_PlayMusic(sdlBgm, 0);
  else
    Mix_FadeInMusic(sdlBgm, 0, time);
  Mix_RewindMusic();
  if (bgmPosition) {
    bgmPosition = (bgmPosition / 2000 * 2000);
    Mix_SetMusicPosition(bgmPosition / 1000);
  }
  
  return Qnil;
}

static VALUE Audio_play_se(int argc, VALUE* argv, VALUE self)
{
  VALUE rbPath, rbOptions;
  rb_scan_args(argc, argv, "11", &rbPath, &rbOptions);
  if (NIL_P(rbOptions))
    rbOptions = rb_hash_new();

  VALUE rbCompletePath = GetCompletePath(rbPath, true);
  char* path = StringValuePtr(rbCompletePath);
  Mix_Chunk* sdlSE = Mix_LoadWAV(path);
  if (!sdlSE)
    rb_raise_sdl_mix_error();

  int panning = 0;
  int time    = 0;
  int volume  = 255;

  VALUE val;
  Check_Type(rbOptions, T_HASH);
  if (!NIL_P(val = rb_hash_aref(rbOptions, symbol_panning)))
    panning = NORMALIZE(NUM2INT(val), -255, 255);
  if (!NIL_P(val = rb_hash_aref(rbOptions, symbol_time)))
    time = NUM2INT(val);
  if (!NIL_P(val = rb_hash_aref(rbOptions, symbol_volume)))
    volume = NORMALIZE(NUM2INT(val), 0, 255);
  
  int sdlChannel;
  if (time <= 50)
    sdlChannel = Mix_PlayChannel(-1, sdlSE, 0);
  else
    sdlChannel = Mix_FadeInChannel(-1, sdlSE, 0, time);
  if (sdlChannel == -1)
    return Qnil;

  Mix_Volume(sdlChannel, DIV255(volume * MIX_MAX_VOLUME));
  int sdlLeftPanning  = 255;
  int sdlRightPanning = 255;
  if (panning < 0) {
    sdlRightPanning = 255 - (-panning);
  } else if (0 < panning) {
    sdlLeftPanning = 255 - panning;
  }
  if (!Mix_SetPanning(sdlChannel, sdlLeftPanning, sdlRightPanning)) {
    rb_raise_sdl_mix_error();
  }

  return Qnil;
}

static VALUE Audio_playing_bgm(VALUE self)
{
  return Mix_PlayingMusic() ? Qtrue : Qfalse;
}

static VALUE Audio_stop_all_ses(int argc, VALUE* argv, VALUE self)
{
  VALUE rbOptions;
  rb_scan_args(argc, argv, "01", &rbOptions);
  if (NIL_P(rbOptions))
    rbOptions = rb_hash_new();

  int time = 0;
  
  Check_Type(rbOptions, T_HASH);
  VALUE val;
  if (!NIL_P(val = rb_hash_aref(rbOptions, symbol_time)))
    time = NUM2INT(val);
  
  if (time <= 50)
    Mix_HaltChannel(-1);
  else
    Mix_FadeOutChannel(-1, time);
  
  return Qnil;
}

static VALUE Audio_stop_bgm(int argc, VALUE* argv, VALUE self)
{
  VALUE rbOptions;
  rb_scan_args(argc, argv, "01", &rbOptions);
  if (NIL_P(rbOptions))
    rbOptions = rb_hash_new();

  int time = 0;
  
  Check_Type(rbOptions, T_HASH);
  VALUE val;
  if (!NIL_P(val = rb_hash_aref(rbOptions, symbol_time)))
    time = NUM2INT(val);
  
  if (time <= 50)
    Mix_HaltMusic();
  else
    Mix_FadeOutMusic(time);
  sdlBgm = NULL;
  
  return Qnil;
}

void SdlMusicFinished(void)
{
  if (sdlBgm && bgmLoop) {
    bgmPosition = 0;
    Mix_PlayMusic(sdlBgm, 0);
  }
}

void InitializeSdlAudio(void)
{
  if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 1024) == -1)
    rb_raise_sdl_mix_error();
  Mix_AllocateChannels(8);
  Mix_HookMusicFinished(SdlMusicFinished);
  Audio_bgm_volume_eq(rb_mAudio, INT2NUM(255));
}

void InitializeAudio(void)
{
  rb_mAudio = rb_define_module_under(rb_mStarRuby, "Audio");
  rb_define_singleton_method(rb_mAudio, "bgm_position",
                             Audio_bgm_position,    0);
  rb_define_singleton_method(rb_mAudio, "bgm_volume",   Audio_bgm_volume,    0);
  rb_define_singleton_method(rb_mAudio, "bgm_volume=",  Audio_bgm_volume_eq, 1);
  rb_define_singleton_method(rb_mAudio, "play_bgm",     Audio_play_bgm,     -1);
  rb_define_singleton_method(rb_mAudio, "play_se",      Audio_play_se,      -1);
  rb_define_singleton_method(rb_mAudio, "playing_bgm?", Audio_playing_bgm,   0);
  rb_define_singleton_method(rb_mAudio, "stop_all_ses", Audio_stop_all_ses, -1);
  rb_define_singleton_method(rb_mAudio, "stop_bgm",     Audio_stop_bgm,     -1);

  symbol_loop     = ID2SYM(rb_intern("loop"));
  symbol_panning  = ID2SYM(rb_intern("panning"));
  symbol_position = ID2SYM(rb_intern("position"));
  symbol_time     = ID2SYM(rb_intern("time"));
  symbol_volume   = ID2SYM(rb_intern("volume"));
}

void UpdateAudio(void)
{
  Uint32 now = SDL_GetTicks();
  if (Mix_PlayingMusic())
    bgmPosition += now - sdlPreviousTicks;
  else
    bgmPosition = 0;
  sdlPreviousTicks = now;
}
