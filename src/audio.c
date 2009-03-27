#include "starruby_private.h"

#define MAX_CHANNEL_COUNT (8)

static bool isEnabled = false;
static bool bgmLoop = false;
static uint8_t bgmVolume = 255;
static Mix_Music* sdlBgm = NULL;
static Uint32 sdlBgmStartTicks = 0;
static Uint32 sdlBgmLastPausedPosition = 0;

static volatile VALUE rbChunkCache = Qundef;
static volatile VALUE rbMusicCache = Qundef;

static volatile VALUE symbol_loop     = Qundef;
static volatile VALUE symbol_panning  = Qundef;
static volatile VALUE symbol_position = Qundef;
static volatile VALUE symbol_time     = Qundef;
static volatile VALUE symbol_volume   = Qundef;

static VALUE
Audio_bgm_position(VALUE self)
{
  if (isEnabled) {
    if (Mix_PlayingMusic()) {
      sdlBgmLastPausedPosition = SDL_GetTicks() - sdlBgmStartTicks;
    }
    return LONG2NUM(sdlBgmLastPausedPosition);
  } else {
    return Qnil;
  }
}

static VALUE
Audio_bgm_volume(VALUE self)
{
  return INT2FIX(bgmVolume);
}

static VALUE
Audio_bgm_volume_eq(VALUE self, VALUE rbVolume)
{
  int tmpBgmVolume = NUM2INT(rbVolume);
  if (tmpBgmVolume < 0 || 256 <= tmpBgmVolume) {
    rb_raise(rb_eArgError, "invalid bgm volume: %d", bgmVolume);
  }
  bgmVolume = tmpBgmVolume;
  if (isEnabled) {
    Mix_VolumeMusic(DIV255((int)(bgmVolume * MIX_MAX_VOLUME)));
  }
  return INT2FIX(bgmVolume);
}

static VALUE
Audio_play_bgm(int argc, VALUE* argv, VALUE self)
{
  volatile VALUE rbPath, rbOptions;
  rb_scan_args(argc, argv, "11", &rbPath, &rbOptions);
  if (NIL_P(rbOptions)) {
    rbOptions = rb_hash_new();
  }
  Check_Type(rbOptions, T_HASH);

  volatile VALUE rbCompletePath = strb_GetCompletePath(rbPath, true);
  volatile VALUE val;
  if (!NIL_P(val = rb_hash_aref(rbMusicCache, rbCompletePath))) {
    sdlBgm = (Mix_Music*)NUM2LONG(val);
  } else {
    const char* path = StringValueCStr(rbCompletePath);
    if (!(sdlBgm = Mix_LoadMUS(path))) {
      rb_raise_sdl_mix_error();
    }
    rb_hash_aset(rbMusicCache, rbCompletePath, ULONG2NUM((unsigned long)sdlBgm));
  }

  int time         = 0;
  int volume       = 255;
  long bgmPosition = 0;
  bgmLoop = RTEST(rb_hash_aref(rbOptions, symbol_loop));
  if (!NIL_P(val = rb_hash_aref(rbOptions, symbol_position))) {
    bgmPosition = MAX(NUM2LONG(val), 0);
  }
  if (!NIL_P(val = rb_hash_aref(rbOptions, symbol_time))) {
    time = NUM2INT(val);
  }
  if (bgmPosition) {
    time = MAX(time, 250);
  } else if (time < 250) {
    time = 0;
  }
  if (!NIL_P(val = rb_hash_aref(rbOptions, symbol_volume))) {
    volume = NUM2INT(val);
    if (volume < 0 || 256 <= volume) {
      rb_raise(rb_eArgError, "invalid volume: %d", volume);
    }
  }
  Audio_bgm_volume_eq(self, INT2FIX(volume));
  if (!isEnabled) {
    return Qnil;
  }
  //printf("time: %d, bgmPosition: %d\n", time, bgmPosition);
  if (Mix_FadeInMusicPos(sdlBgm, 0, time, bgmPosition)) {
    rb_raise_sdl_mix_error();
  }
  sdlBgmStartTicks = SDL_GetTicks() - bgmPosition;
  
  return Qnil;
}

static VALUE
Audio_play_se(int argc, VALUE* argv, VALUE self)
{
  volatile VALUE rbPath, rbOptions;
  rb_scan_args(argc, argv, "11", &rbPath, &rbOptions);
  if (NIL_P(rbOptions)) {
    rbOptions = rb_hash_new();
  }
  Check_Type(rbOptions, T_HASH);

  volatile VALUE rbCompletePath = strb_GetCompletePath(rbPath, true);
  Mix_Chunk* sdlSE = NULL;
  volatile VALUE val;
  if (isEnabled) {
    if (!NIL_P(val = rb_hash_aref(rbChunkCache, rbCompletePath))) {
      sdlSE = (Mix_Chunk*)NUM2ULONG(val);
    } else {
      const char* path = StringValueCStr(rbCompletePath);
      if (!(sdlSE = Mix_LoadWAV(path))) {
        rb_raise_sdl_mix_error();
      }
      rb_hash_aset(rbChunkCache, rbCompletePath, ULONG2NUM((unsigned long)sdlSE));
    }
  }

  int panning = 0;
  int time    = 0;
  int volume  = 255;
  if (!NIL_P(val = rb_hash_aref(rbOptions, symbol_panning))) {
    panning = NUM2INT(val);
    if (panning <= -256 || 256 <= panning) {
      rb_raise(rb_eArgError, "invalid panning: %d", panning);
    }
  }
  if (!NIL_P(val = rb_hash_aref(rbOptions, symbol_time))) {
    time = NUM2INT(val);
  }
  if (!NIL_P(val = rb_hash_aref(rbOptions, symbol_volume))) {
    volume = NUM2INT(val);
    if (volume < 0 || 256 <= volume) {
      rb_raise(rb_eArgError, "invalid volume: %d", volume);
    }
  }
  if (!isEnabled) {
    return Qnil;
  }
  int sdlChannel;
  if (time < 250) {
    sdlChannel = Mix_PlayChannel(-1, sdlSE, 0);
  } else {
    sdlChannel = Mix_FadeInChannel(-1, sdlSE, 0, time);
  }
  if (sdlChannel == -1) {
    return Qnil;
  }
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

static VALUE
Audio_playing_bgm(VALUE self)
{
  return (isEnabled &&
          (Mix_PlayingMusic() ||
           Mix_FadingMusic() != MIX_NO_FADING)) ? Qtrue : Qfalse;
}

static VALUE
Audio_playing_se_count(VALUE self)
{
  if (!isEnabled) {
    return INT2FIX(0);
  }
  return INT2FIX(Mix_Playing(-1));
}

static VALUE
Audio_stop_all_ses(int argc, VALUE* argv, VALUE self)
{
  volatile VALUE rbOptions;
  rb_scan_args(argc, argv, "01", &rbOptions);
  if (NIL_P(rbOptions)) {
    rbOptions = rb_hash_new();
  }
  Check_Type(rbOptions, T_HASH);

  int time = 0;
  volatile VALUE val;
  if (!NIL_P(val = rb_hash_aref(rbOptions, symbol_time))) {
    time = NUM2INT(val);
  }
  if (!isEnabled) {
    return Qnil;
  }
  if (time <= 250) {
    Mix_HaltChannel(-1);
  } else {
    Mix_FadeOutChannel(-1, time);
  }
  return Qnil;
}

static VALUE
Audio_stop_bgm(int argc, VALUE* argv, VALUE self)
{
  volatile VALUE rbOptions;
  rb_scan_args(argc, argv, "01", &rbOptions);
  if (NIL_P(rbOptions)) {
    rbOptions = rb_hash_new();
  }
  Check_Type(rbOptions, T_HASH);

  if (!isEnabled) {
    return Qnil;
  }
  int time = 0;
  volatile VALUE val;
  if (!NIL_P(val = rb_hash_aref(rbOptions, symbol_time))) {
    time = NUM2INT(val);
  }
  time = MAX(time, 0);
  if (time < 250) {
    Mix_HaltMusic();
  } else {
    Mix_FadeOutMusic(time);
  }
  sdlBgm = NULL;
  return Qnil;
}

static void
SdlMusicFinished(void)
{
  if (sdlBgm && bgmLoop) {
    sdlBgmStartTicks = SDL_GetTicks();
    if (isEnabled && Mix_PlayMusic(sdlBgm, 0)) {
      rb_raise_sdl_mix_error();
    }
  }
}

void
strb_InitializeSdlAudio(void)
{
  if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 1024)) {
    rb_io_puts(1, (VALUE[]) {rb_str_new2(Mix_GetError())}, rb_stderr);
    isEnabled = false;
  } else {
    Mix_AllocateChannels(MAX_CHANNEL_COUNT);
    Mix_HookMusicFinished(SdlMusicFinished);
    isEnabled = true;
  }
}

VALUE
strb_InitializeAudio(VALUE rb_mStarRuby)
{
  VALUE rb_mAudio = rb_define_module_under(rb_mStarRuby, "Audio");
  rb_define_module_function(rb_mAudio, "bgm_position",
                            Audio_bgm_position, 0);
  rb_define_module_function(rb_mAudio, "bgm_volume",
                            Audio_bgm_volume, 0);
  rb_define_module_function(rb_mAudio, "bgm_volume=",
                            Audio_bgm_volume_eq, 1);
  rb_define_module_function(rb_mAudio, "play_bgm",
                            Audio_play_bgm, -1);
  rb_define_module_function(rb_mAudio, "play_se",
                            Audio_play_se, -1);
  rb_define_module_function(rb_mAudio, "playing_bgm?",
                            Audio_playing_bgm, 0);
  rb_define_module_function(rb_mAudio, "playing_se_count",
                            Audio_playing_se_count, 0);
  rb_define_module_function(rb_mAudio, "stop_all_ses",
                            Audio_stop_all_ses, -1);
  rb_define_module_function(rb_mAudio, "stop_bgm",
                            Audio_stop_bgm, -1);

  rb_define_const(rb_mAudio, "MAX_SE_COUNT", INT2FIX(MAX_CHANNEL_COUNT));

  symbol_loop     = ID2SYM(rb_intern("loop"));
  symbol_panning  = ID2SYM(rb_intern("panning"));
  symbol_position = ID2SYM(rb_intern("position"));
  symbol_time     = ID2SYM(rb_intern("time"));
  symbol_volume   = ID2SYM(rb_intern("volume"));

  Audio_bgm_volume_eq(rb_mAudio, INT2FIX(255));

  rbMusicCache = rb_iv_set(rb_mAudio, "music_cache", rb_hash_new());
  rbChunkCache = rb_iv_set(rb_mAudio, "chunk_cache", rb_hash_new());

  return rb_mAudio;
}

static int
FreeChunkCacheItem(VALUE rbKey, VALUE rbValue)
{
  Mix_Chunk* chunk = (Mix_Chunk*)NUM2ULONG(rbValue);
  if (chunk) {
    Mix_FreeChunk(chunk);
  }
  return ST_CONTINUE;
}

static int
FreeMusicCacheItem(VALUE rbKey, VALUE rbValue)
{
  Mix_Music* music = (Mix_Music*)NUM2ULONG(rbValue);
  if (music) {
    Mix_FreeMusic(music);
  }
  return ST_CONTINUE;
}

void
strb_FinalizeAudio(void)
{
  rb_hash_foreach(rbChunkCache, FreeChunkCacheItem, 0);
  rb_hash_foreach(rbMusicCache, FreeMusicCacheItem, 0);
  Mix_CloseAudio();
}
