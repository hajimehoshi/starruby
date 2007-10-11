#include "starruby.h"

static VALUE rb_eStarRubyError;

void InitializeStarRubyError(void)
{
  rb_eStarRubyError = rb_define_class("StarRubyError", rb_eStandardError);
}

inline void rb_raise_star_ruby_error(char* message)
{
  rb_raise(rb_eStarRubyError, message);
}

inline void rb_raise_sdl_error(void)
{
  rb_raise(rb_eStarRubyError, SDL_GetError());
}
