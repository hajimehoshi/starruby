#include "starruby.h"

VALUE rb_mStarRuby;
VALUE rb_eStarRubyError;

void Init_starruby(void)
{
  rb_mStarRuby = rb_define_module("StarRuby");

  rb_eStarRubyError = rb_define_class("StarRubyError", rb_eStandardError);
  
  init_game();
  init_screen();
  init_texture();
}

inline void rb_raise_sdl_error(void)
{
  rb_raise(rb_eStarRubyError, SDL_GetError());
}
