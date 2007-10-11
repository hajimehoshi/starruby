#include "starruby.h"

static VALUE texture_initialize(VALUE self, VALUE rbWidth, VALUE rbHeight)
{
  rb_iv_set(self, "width", rbWidth);
  rb_iv_set(self, "height", rbHeight);
  return Qnil;
}

static VALUE texture_height(VALUE self)
{
  return rb_iv_get(self, "height");
}

static VALUE texture_size(VALUE self)
{
  VALUE rbSize = rb_assoc_new(
    rb_iv_get(self, "width"),
    rb_iv_get(self, "height"));
  rb_obj_freeze(rbSize);
  return rbSize;
}

static VALUE texture_width(VALUE self)
{
  return rb_iv_get(self, "width");
}

void InitializeTexture(void)
{
  VALUE rb_cTexture = rb_define_class_under(rb_mStarRuby, "Texture", rb_cObject);
  rb_define_private_method(rb_cTexture, "initialize", texture_initialize, 2);
  rb_define_method(rb_cTexture, "height", texture_height, 0);
  rb_define_method(rb_cTexture, "size",   texture_size,   0);
  rb_define_method(rb_cTexture, "width",  texture_width,  0);
}
