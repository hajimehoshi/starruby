#include "starruby.h"

VALUE rb_mStarRuby;

void Init_starruby(void)
{
  rb_mStarRuby = rb_define_module("StarRuby");
  
  InitializeGame();
  InitializeScreen();
  InitializeStarRubyError();
  InitializeTexture();
}
