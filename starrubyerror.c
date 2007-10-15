#include "starruby.h"

VALUE rb_eStarRubyError;

void InitializeStarRubyError(void)
{
  rb_eStarRubyError = rb_define_class_under(rb_mStarRuby, "StarRubyError",
                                            rb_eStandardError);
}
