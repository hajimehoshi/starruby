#include "starruby.h"
#include "starruby_private.h"
#include "st.h"

#define MAX_CHANNEL_COUNT (8)

static bool isEnabled = false;
static bool bgmLoop = false;
static Uint32 bgmPosition = 0;
static int bgmVolume = 255;
static Mix_Music* sdlBgm = NULL;
static Uint32 sdlPreviousTicks = 0;

static volatile VALUE rbChunkCache;
static volatile VALUE rbMusicCache;

static volatile VALUE symbol_loop;
static volatile VALUE symbol_panning;
static volatile VALUE symbol_position;
static volatile VALUE symbol_time;
static volatile VALUE symbol_volume;

static VALUE
Audio_bgm_position(VALUE self)
{
  return LONG2NUM(bgmPosition);
}

static VALUE
Audio_bgm_volume(VALUE self)
{
  return INT2NUM(bgmVolume);
}

static VALUE
Audio_bgm_volume_eq(VALUE self, VALUE rbVolume)
{
  bgmVolume = NUM2INT(rbVolume);
  if (bgmVolume < 0 || 256 <= bgmVolume)
    rb_raise(rb_eArgError, "invalid bgm volume: %d", bgmVolume);
  if (isEnabled)
    Mix_VolumeMusic(DIV255((int)(bgmVolume * MIX_MAX_VOLUME)));
  return INT2NUM(bgmVolume);
}

static VALUE
Audio_play_bgm(int argc, VALUE* argv, VALUE self)
{
  volatile VALUE rbPath, rbOptions;
  rb_scan_args(argc, argv, "11", &rbPath, &rbOptions);
  if (NIL_P(rbOptions))
    rbOptions = rb_hash_new();

  volatile VALUE rbCompletePath = GetCompletePath(rbPath, true);
  volatile VALUE val;
  if (!NIL_P(val = rb_hash_aref(rbMusicCache, rbCompletePath))) {
    sdlBgm = (Mix_Music*)NUM2LONG(val);
  } else {
    char* path = StringValuePtr(rbCompletePath);
    if (!(sdlBgm = Mix_LoadMUS(path)))
      rb_raise_sdl_mix_error();
    rb_hash_aset(rbMusicCache, rbCompletePath, ULONG2NUM((unsigned long)sdlBgm));
  }

  int time   = 0;
  int volume = 255;

  Check_Type(rbOptions, T_HASH);
  bgmLoop = RTEST(rb_hash_aref(rbOptions, symbol_loop));
  if (!NIL_P(val = rb_hash_aref(rbOptions, symbol_position)))
    bgmPosition = MAX(NUM2INT(val), 0);
  if (!NIL_P(val = rb_hash_aref(rbOptions, symbol_time)))
    time = NUM2INT(val);
  if (!NIL_P(val = rb_hash_aref(rbOptions, symbol_volume))) {
    volume = NUM2INT(val);
    if (volume < 0 || 256 <= volume)
      rb_raise(rb_eArgError, "invalid volume: %d", volume);
  }

  Audio_bgm_volume_eq(self, INT2NUM(volume));
  if (!isEnabled)
    return Qnil;
  if (time <= 50) {
    if (Mix_PlayMusic(sdlBgm, 0))
      rb_raise_sdl_mix_error();
  } else {
    if (Mix_FadeInMusic(sdlBgm, 0, time))
      rb_raise_sdl_mix_error();
  }
  Mix_RewindMusic();
  if (bgmPosition) {
    bgmPosition = (bgmPosition / 2000 * 2000);
    if (Mix_SetMusicPosition(bgmPosition / 1000))
      rb_raise_sdl_mix_error();
  }
  
  return Qnil;
}

static VALUE
Audio_play_se(int argc, VALUE* argv, VALUE self)
{
  volatile VALUE rbPath, rbOptions;
  rb_scan_args(argc, argv, "11", &rbPath, &rbOptions);
  if (NIL_P(rbOptions))
    rbOptions = rb_hash_new();

  volatile VALUE rbCompletePath = GetCompletePath(rbPath, true);
  Mix_Chunk* sdlSE = NULL;
  volatile VALUE val;
  if (isEnabled) {
    if (!NIL_P(val = rb_hash_aref(rbChunkCache, rbCompletePath))) {
      sdlSE = (Mix_Chunk*)NUM2ULONG(val);
    } else {
      char* path = StringValuePtr(rbCompletePath);
      if (!(sdlSE = Mix_LoadWAV(path)))
	rb_raise_sdl_mix_error();
      rb_hash_aset(rbChunkCache, rbCompletePath, ULONG2NUM((unsigned long)sdlSE));
    }
  }

  int panning = 0;
  int time    = 0;
  int volume  = 255;

  Check_Type(rbOptions, T_HASH);
  if (!NIL_P(val = rb_hash_aref(rbOptions, symbol_panning))) {
    panning = NUM2INT(val);
    if (panning <= -256 || 256 <= panning)
      rb_raise(rb_eArgError, "invalid panning: %d", panning);
  }
  if (!NIL_P(val = rb_hash_aref(rbOptions, symbol_time))) 
    time = NUM2INT(val);
  if (!NIL_P(val = rb_hash_aref(rbOptions, symbol_volume))) {
    volume = NUM2INT(val);
    if (volume < 0 || 256 <= volume)
      rb_raise(rb_eArgError, "invalid volume: %d", volume);
  }

  if (!isEnabled)
    return Qnil;
  
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
  if (!Mix_SetPanning(sdlChannel, sdlLeftPanning, sdlRightPanning))
    rb_raise_sdl_mix_error();

  return Qnil;
}

static VALUE
Audio_playing_bgm(VALUE self)
{
  return (isEnabled && Mix_PlayingMusic()) ? Qtrue : Qfalse;
}

static VALUE
Audio_playing_se_count(VALUE self)
{
  if (!isEnabled)
    return INT2NUM(0);
  return INT2NUM(Mix_Playing(-1));
}

static VALUE
Audio_stop_all_ses(int argc, VALUE* argv, VALUE self)
{
  volatile VALUE rbOptions;
  rb_scan_args(argc, argv, "01", &rbOptions);
  if (NIL_P(rbOptions))
    rbOptions = rb_hash_new();

  int time = 0;
  
  Check_Type(rbOptions, T_HASH);
  volatile VALUE val;
  if (!NIL_P(val = rb_hash_aref(rbOptions, symbol_time)))
    time = NUM2INT(val);

  if (!isEnabled)
    return Qnil;
  
  if (time <= 50)
    Mix_HaltChannel(-1);
  else
    Mix_FadeOutChannel(-1, time);
  
  return Qnil;
}

static VALUE
Audio_stop_bgm(int argc, VALUE* argv, VALUE self)
{
  volatile VALUE rbOptions;
  rb_scan_args(argc, argv, "01", &rbOptions);
  if (NIL_P(rbOptions))
    rbOptions = rb_hash_new();

  int time = 0;
  
  Check_Type(rbOptions, T_HASH);
  volatile VALUE val;
  if (!NIL_P(val = rb_hash_aref(rbOptions, symbol_time)))
    time = NUM2INT(val);

  if (!isEnabled)
    return Qnil;
  
  if (time <= 50)
    Mix_HaltMusic();
  else
    Mix_FadeOutMusic(time);
  sdlBgm = NULL;
  
  return Qnil;
}

void
SdlMusicFinished(void)
{
  if (sdlBgm && bgmLoop) {
    bgmPosition = 0;
    if (isEnabled)
      if (Mix_PlayMusic(sdlBgm, 0))
	rb_raise_sdl_mix_error();
  }
}

void
InitializeSdlAudio(void)
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

void
InitializeAudio(void)
{
  rb_mAudio = rb_define_module_under(rb_mStarRuby, "Audio");
  rb_define_module_function(rb_mAudio, "bgm_position",
                            Audio_bgm_position,     0);
  rb_define_module_function(rb_mAudio, "bgm_volume",
                            Audio_bgm_volume,       0);
  rb_define_module_function(rb_mAudio, "bgm_volume=",
                            Audio_bgm_volume_eq,    1);
  rb_define_module_function(rb_mAudio, "play_bgm",
                            Audio_play_bgm,         -1);
  rb_define_module_function(rb_mAudio, "play_se",
                            Audio_play_se,          -1);
  rb_define_module_function(rb_mAudio, "playing_bgm?",
                            Audio_playing_bgm,      0);
  rb_define_module_function(rb_mAudio, "playing_se_count",
                            Audio_playing_se_count, 0);
  rb_define_module_function(rb_mAudio, "stop_all_ses",
                            Audio_stop_all_ses,     -1);
  rb_define_module_function(rb_mAudio, "stop_bgm",
                            Audio_stop_bgm,         -1);

  rb_define_const(rb_mAudio, "MAX_SE_COUNT", INT2NUM(MAX_CHANNEL_COUNT));

  symbol_loop     = ID2SYM(rb_intern("loop"));
  symbol_panning  = ID2SYM(rb_intern("panning"));
  symbol_position = ID2SYM(rb_intern("position"));
  symbol_time     = ID2SYM(rb_intern("time"));
  symbol_volume   = ID2SYM(rb_intern("volume"));

  Audio_bgm_volume_eq(rb_mAudio, INT2NUM(255));

  rbMusicCache = rb_iv_set(rb_mAudio, "music_cache", rb_hash_new());
  rbChunkCache = rb_iv_set(rb_mAudio, "chunk_cache", rb_hash_new());
}

void
UpdateAudio(void)
{
  Uint32 now = SDL_GetTicks();
  if (isEnabled && Mix_PlayingMusic())
    bgmPosition += now - sdlPreviousTicks;
  else
    bgmPosition = 0;
  sdlPreviousTicks = now;
}

static int
FreeChunkCacheItem(VALUE rbKey, VALUE rbValue)
{
  Mix_Chunk* chunk = (Mix_Chunk*)NUM2ULONG(rbValue);
  if (chunk)
    Mix_FreeChunk(chunk);
  return ST_CONTINUE;
}

static int
FreeMusicCacheItem(VALUE rbKey, VALUE rbValue)
{
  Mix_Music* music = (Mix_Music*)NUM2ULONG(rbValue);
  if (music)
    Mix_FreeMusic(music);
  return ST_CONTINUE;
}

void
FinalizeAudio(void)
{
  rb_hash_foreach(rbChunkCache, FreeChunkCacheItem, 0);
  rb_hash_foreach(rbMusicCache, FreeMusicCacheItem, 0);
  Mix_CloseAudio();
}
