#include "starruby.h"

static VALUE rbScreenSize = Qnil;

static VALUE Screen_height()
{
  return INT2NUM(SCREEN_HEIGHT);
}

static VALUE Screen_offscreen()
{
  return Qnil;
}

static VALUE Screen_size()
{
  if (rbScreenSize == Qnil) {
    rbScreenSize = rb_assoc_new(INT2NUM(SCREEN_WIDTH), INT2NUM(SCREEN_HEIGHT));
    rb_obj_freeze(rbScreenSize);
  }
  return rbScreenSize;
}

static VALUE Screen_width()
{
  return INT2NUM(SCREEN_WIDTH);
}

void InitializeScreen(void)
{
  VALUE rb_mScreen = rb_define_module_under(rb_mStarRuby, "Screen");
  rb_define_singleton_method(rb_mScreen, "height",    Screen_height,    0);
  rb_define_singleton_method(rb_mScreen, "offscreen", Screen_offscreen, 0);
  rb_define_singleton_method(rb_mScreen, "size",      Screen_size,      0);
  rb_define_singleton_method(rb_mScreen, "width",     Screen_width,     0);
}
