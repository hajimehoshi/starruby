#include "starruby.h"

static VALUE rb_mScreen;
static VALUE rbOffscreenTexture = Qnil;
static VALUE rbScreenSize = Qnil;

static VALUE Screen_height(VALUE self)
{
  return INT2NUM(SCREEN_HEIGHT);
}

static VALUE Screen_offscreen(VALUE self)
{
  VALUE rbTexture = rb_iv_get(rb_mScreen, "offscreen_texture");;
  if (rbTexture == Qnil) {
    VALUE rbOffscreenTexture = rb_funcall(rb_cTexture, rb_intern("new"), 2,
                                          INT2NUM(SCREEN_WIDTH),
                                          INT2NUM(SCREEN_HEIGHT));
    return rb_iv_set(rb_mScreen, "offscreen_texture", rbOffscreenTexture);
  } else {
    return rbTexture;
  }
}

static VALUE Screen_size(VALUE self)
{
  if (rbScreenSize == Qnil) {
    rbScreenSize = rb_assoc_new(INT2NUM(SCREEN_WIDTH), INT2NUM(SCREEN_HEIGHT));
    rb_obj_freeze(rbScreenSize);
  }
  return rbScreenSize;
}

static VALUE Screen_width(VALUE self)
{
  return INT2NUM(SCREEN_WIDTH);
}

void InitializeScreen(void)
{
  rb_mScreen = rb_define_module_under(rb_mStarRuby, "Screen");
  rb_define_singleton_method(rb_mScreen, "height",    Screen_height,    0);
  rb_define_singleton_method(rb_mScreen, "offscreen", Screen_offscreen, 0);
  rb_define_singleton_method(rb_mScreen, "size",      Screen_size,      0);
  rb_define_singleton_method(rb_mScreen, "width",     Screen_width,     0);
}
