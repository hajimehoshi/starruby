#include "starruby.h"

void InitializeStarRubyError(void)
{
  rb_eStarRubyError = rb_define_class_under(rb_mStarRuby, "StarRubyError",
                                            rb_eStandardError);
}
