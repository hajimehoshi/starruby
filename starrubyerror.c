#include "starruby.h"

VALUE rb_eStarRubyError;

void InitializeStarRubyError(void)
{
  rb_eStarRubyError = rb_define_class("StarRubyError", rb_eStandardError);
}
