#include "starruby.h"

static const int SCREEN_WIDTH = 320;
static const int SCREEN_HEIGHT = 240;
static VALUE rbScreenSize = Qnil;

static VALUE screen_height()
{
  return INT2NUM(SCREEN_HEIGHT);
}

static VALUE screen_offscreen()
{
  return Qnil;
}

static VALUE screen_size()
{
  if (rbScreenSize == Qnil) {
    rbScreenSize = rb_assoc_new(INT2NUM(SCREEN_WIDTH), INT2NUM(SCREEN_HEIGHT));
    rb_obj_freeze(rbScreenSize);
  }
  return rbScreenSize;
}

static VALUE screen_width()
{
  return INT2NUM(SCREEN_WIDTH);
}

void InitializeScreen(void)
{
  VALUE rb_mScreen = rb_define_module_under(rb_mStarRuby, "Screen");
  rb_define_singleton_method(rb_mScreen, "height",    screen_height,    0);
  rb_define_singleton_method(rb_mScreen, "offscreen", screen_offscreen, 0);
  rb_define_singleton_method(rb_mScreen, "size",      screen_size,      0);
  rb_define_singleton_method(rb_mScreen, "width",     screen_width,     0);
}
