#include "starruby.h"

static VALUE symbol_loop;
static VALUE symbol_panning;
static VALUE symbol_time;
static VALUE symbol_volume;

static VALUE Audio_bgm_position(VALUE self)
{
  return Qnil;
}

static VALUE Audio_bgm_position_eq(VALUE self, VALUE rbPosition)
{
  return Qnil;
}

static VALUE Audio_bgm_volume(VALUE self)
{
  return rb_iv_get(rb_mAudio, "volume");
}

static VALUE Audio_bgm_volume_eq(VALUE self, VALUE rbVolume)
{
  int volume = NORMALIZE(NUM2INT(rbVolume), 0, 255);
  rb_iv_set(rb_mAudio, "volume", INT2NUM(volume));
  int sdlVolume = DIV255((int)(volume * MIX_MAX_VOLUME));
  Mix_VolumeMusic(sdlVolume);
  return INT2NUM(volume);
}

static VALUE Audio_play_bgm(int argc, VALUE* argv, VALUE self)
{
  VALUE rbPath;
  VALUE rbOptions;
  rb_scan_args(argc, argv, "11", &rbPath, &rbOptions);
  if (NIL_P(rbOptions))
    rbOptions = rb_hash_new();

  VALUE rbCompletePath = GetCompletePath(rbPath);
  char* path = StringValuePtr(rbCompletePath);
  Mix_Music* sdlBgm = Mix_LoadMUS(path);
  if (!sdlBgm)
    rb_raise_sdl_mix_error();

  bool loop   = false;
  int time    = 0;

  VALUE val;
  st_table* table = RHASH(rbOptions)->tbl;
  if (st_lookup(table, symbol_loop, &val))
    loop = RTEST(val);
  if (st_lookup(table, symbol_time, &val))
    time = NUM2INT(val);

  if (time == 0)
    Mix_PlayMusic(sdlBgm, loop ? -1 : 0);
  else
    Mix_FadeInMusic(sdlBgm, loop ? -1 : 0, time);
  
  return Qnil;
}

static VALUE Audio_play_se(int argc, VALUE* argv, VALUE self)
{
  VALUE rbPath;
  VALUE rbOptions;
  rb_scan_args(argc, argv, "11", &rbPath, &rbOptions);
  if (NIL_P(rbOptions))
    rbOptions = rb_hash_new();

  VALUE rbCompletePath = GetCompletePath(rbPath);
  char* path = StringValuePtr(rbCompletePath);
  Mix_Chunk* sdlSE = Mix_LoadWAV(path);
  if (!sdlSE)
    rb_raise_sdl_mix_error();

  int panning = 127;
  int time    = 0;
  int volume  = 255;

  VALUE val;
  st_table* table = RHASH(rbOptions)->tbl;
  if (st_lookup(table, symbol_panning, &val))
    panning = NORMALIZE(NUM2INT(val), 0, 255);
  if (st_lookup(table, symbol_time, &val))
    time = NUM2INT(val);
  if (st_lookup(table, symbol_volume, &val))
    volume = NORMALIZE(NUM2INT(val), 0, 255);

  int sdlChannel;
  if (time == 0)
    sdlChannel = Mix_PlayChannel(-1, sdlSE, 0);
  else
    sdlChannel = Mix_FadeInChannel(-1, sdlSE, 0, time);
  if (sdlChannel == -1)
    return Qnil;

  Mix_Volume(sdlChannel, DIV255(volume * MIX_MAX_VOLUME));
  int sdlLeftPanning  = 255;
  int sdlRightPanning = 255;
  if (panning < 127)
    sdlRightPanning = panning * 2;
  else if (128 < panning)
    sdlLeftPanning = 254 - (panning - 128) * 2;
  Mix_SetPanning(sdlChannel, sdlLeftPanning, sdlRightPanning);

  return Qnil;
}

static VALUE Audio_stop_all_ses(int argc, VALUE* argv, VALUE self)
{
  VALUE rbOptions;
  rb_scan_args(argc, argv, "01", &rbOptions);
  if (NIL_P(rbOptions))
    rbOptions = rb_hash_new();

  int time = 0;
  
  VALUE val;
  st_table* table = RHASH(rbOptions)->tbl;
  if (st_lookup(table, symbol_time, &val))
    time = NUM2INT(val);

  if (time == 0)
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
  
  VALUE val;
  st_table* table = RHASH(rbOptions)->tbl;
  if (st_lookup(table, symbol_time, &val))
    time = NUM2INT(val);

  if (time == 0)
    Mix_HaltMusic();
  else
    Mix_FadeOutMusic(time);
  
  return Qnil;
}

void InitializeSdlAudio(void)
{
  if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 1024) == -1)
    rb_raise_sdl_mix_error();
  Mix_AllocateChannels(8);
  Audio_bgm_volume_eq(rb_mAudio, INT2NUM(255));
}

void FinalizeSdlAudio(void)
{
  Mix_CloseAudio();
}

void InitializeAudio(void)
{
  rb_mAudio = rb_define_module_under(rb_mStarRuby, "Audio");
  rb_define_singleton_method(rb_mAudio, "bgm_position",
                             Audio_bgm_position,    0);
  rb_define_singleton_method(rb_mAudio, "bgm_position=",
                             Audio_bgm_position_eq, 1);
  rb_define_singleton_method(rb_mAudio, "bgm_volume",   Audio_bgm_volume,    0);
  rb_define_singleton_method(rb_mAudio, "bgm_volume=",  Audio_bgm_volume_eq, 1);
  rb_define_singleton_method(rb_mAudio, "play_bgm",     Audio_play_bgm,     -1);
  rb_define_singleton_method(rb_mAudio, "play_se",      Audio_play_se,      -1);
  rb_define_singleton_method(rb_mAudio, "stop_all_ses", Audio_stop_all_ses, -1);
  rb_define_singleton_method(rb_mAudio, "stop_bgm",     Audio_stop_bgm,     -1);

  symbol_loop    = ID2SYM(rb_intern("loop"));
  symbol_panning = ID2SYM(rb_intern("panning"));
  symbol_time    = ID2SYM(rb_intern("time"));
  symbol_volume  = ID2SYM(rb_intern("volume"));
}
