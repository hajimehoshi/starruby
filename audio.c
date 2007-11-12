#include "starruby.h"

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
  rb_define_singleton_method(rb_mAudio, "bgm_volume",  Audio_bgm_volume,    0);
  rb_define_singleton_method(rb_mAudio, "bgm_volume=", Audio_bgm_volume_eq, 1);
  /*rb_define_singleton_method(rb_mAudio, "play_se",      Audio_play_se,      -1);
  rb_define_singleton_method(rb_mAudio, "stop_all_ses", Audio_stop_all_ses, -1);*/
  
  
}
