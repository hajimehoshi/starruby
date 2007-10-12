#include "starruby.h"

static VALUE Texture_initialize(VALUE self, VALUE rbWidth, VALUE rbHeight)
{
  rb_iv_set(self, "width", rbWidth);
  rb_iv_set(self, "height", rbHeight);
  return Qnil;
}

static VALUE Texture_height(VALUE self)
{
  return rb_iv_get(self, "height");
}

static VALUE Texture_size(VALUE self)
{
  VALUE rbSize = rb_assoc_new(
    rb_iv_get(self, "width"),
    rb_iv_get(self, "height"));
  rb_obj_freeze(rbSize);
  return rbSize;
}

static VALUE Texture_width(VALUE self)
{
  return rb_iv_get(self, "width");
}

void InitializeTexture(void)
{
  VALUE rb_cTexture =
    rb_define_class_under(rb_mStarRuby, "Texture", rb_cObject);
  rb_define_private_method(rb_cTexture, "initialize", Texture_initialize, 2);
  rb_define_method(rb_cTexture, "height", Texture_height, 0);
  rb_define_method(rb_cTexture, "size",   Texture_size,   0);
  rb_define_method(rb_cTexture, "width",  Texture_width,  0);
}
