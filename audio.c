#include "starruby.h"



void InitializeAudio(void)
{
  rb_mAudio = rb_define_module_under(rb_mStarRuby, "Audio");
}
