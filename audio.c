#include "starruby.h"

void InitializeSdlAudio(void)
{
  if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 1024) == -1)
    rb_raise_sdl_mix_error();
  Mix_AllocateChannels(8);
}

void FinalizeSdlAudio(void)
{
  Mix_CloseAudio();
}

void InitializeAudio(void)
{
  rb_mAudio = rb_define_module_under(rb_mStarRuby, "Audio");
}
