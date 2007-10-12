#include "starruby.h"

static VALUE Color_alloc(VALUE klass)
{
  struct Color* color = ALLOC(struct Color);
  return Data_Wrap_Struct(klass, 0, -1, color);
}

static VALUE Color_initialize(int argc, VALUE* argv, VALUE self)
{
  VALUE rbRed;
  VALUE rbGreen;
  VALUE rbBlue;
  VALUE rbAlpha;
  rb_scan_args(argc, argv, "31",
               &rbRed, &rbGreen, &rbBlue, &rbAlpha);

  struct Color* color;
  Data_Get_Struct(self, struct Color, color);
  color->red   = NUM2INT(rbRed);
  color->green = NUM2INT(rbGreen);
  color->blue  = NUM2INT(rbBlue);
  color->alpha = (rbAlpha != Qnil) ? NUM2INT(rbAlpha) : 255;
  return Qnil;
}

static VALUE Color_alpha(VALUE self)
{
  struct Color* color;
  Data_Get_Struct(self, struct Color, color);
  return INT2NUM(color->alpha);
}

static VALUE Color_blue(VALUE self)
{
  struct Color* color;
  Data_Get_Struct(self, struct Color, color);
  return INT2NUM(color->blue);
}

static VALUE Color_eq(VALUE self, VALUE rbOther)
{
  return Qnil;
}

static VALUE Color_green(VALUE self)
{
  struct Color* color;
  Data_Get_Struct(self, struct Color, color);
  return INT2NUM(color->green);
}

static VALUE Color_red(VALUE self)
{
  struct Color* color;
  Data_Get_Struct(self, struct Color, color);
  return INT2NUM(color->red);
}

void InitializeColor(void)
{
  VALUE rb_cColor =
    rb_define_class_under(rb_mStarRuby, "Color", rb_cObject);
  rb_define_alloc_func(rb_cColor, Color_alloc);
  rb_define_private_method(rb_cColor, "initialize", Color_initialize, -1);
  rb_define_method(rb_cColor, "alpha", Color_alpha, 0);
  rb_define_method(rb_cColor, "blue",  Color_blue,  0);
  rb_define_method(rb_cColor, "==",    Color_eq,    0);
  rb_define_method(rb_cColor, "green", Color_green, 0);
  rb_define_method(rb_cColor, "red",   Color_red,   0);
}
