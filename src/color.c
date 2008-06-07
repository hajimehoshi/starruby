#include "st.h"
#include "starruby.h"
#include "starruby_private.h"

static volatile VALUE rb_cColor;
static volatile VALUE rbColorCache;

VALUE
strb_GetColorClass(void)
{
  return rb_cColor;
}

static uint32_t
CalcColorHash(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
{
  // The max value of FIXNUM is 0x3fffffff in 32bit machines.
  return ((alpha >> 6) << 24) |
    ((red ^ ((alpha >> 4) & 0x3)) << 16) |
    ((green ^ ((alpha >> 2) & 0x3)) << 8) |
    (blue ^ (alpha & 0x3));
}

static VALUE
Color_s_new(int argc, VALUE* argv, VALUE self)
{
  volatile VALUE rbRed, rbGreen, rbBlue, rbAlpha;
  rb_scan_args(argc, argv, "31",
               &rbRed, &rbGreen, &rbBlue, &rbAlpha);
  int red   = NUM2INT(rbRed);
  int green = NUM2INT(rbGreen);
  int blue  = NUM2INT(rbBlue);
  int alpha = NIL_P(rbAlpha) ? 255 : NUM2INT(rbAlpha);
  if (red < 0 || 256 <= red || green < 0 || 256 <= green ||
      blue < 0 || 256 <= blue || alpha < 0 || 256 <= alpha)
    rb_raise(rb_eArgError, "invalid color value: (r:%d, g:%d, b:%d, a:%d)",
             red, green, blue, alpha);
  VALUE* cacheP = RARRAY_PTR(rbColorCache);
  int length = RARRAY_LEN(rbColorCache);
  for (int i = 0; i < length; i++) {
    VALUE rbColor = cacheP[i];
    if (NIL_P(rbColor))
      break;
    Color color;
    strb_GetColorFromRubyValue(&color, rbColor);
    if (color.red == red &&
        color.green == green &&
        color.blue == blue &&
        color.alpha == alpha) {
      if (0 < i) {
        rb_ary_delete_at(rbColorCache, i);
        rb_ary_unshift(rbColorCache, rbColor);
      }
      return rbColor;
    }
  }
  VALUE args[] = {
    rbRed, rbGreen, rbBlue, NIL_P(rbAlpha) ? INT2NUM(255) : rbAlpha,
  };
  VALUE rbColor = rb_class_new_instance(sizeof(args) / sizeof(VALUE), args, self);
  rb_ary_pop(rbColorCache);
  rb_ary_unshift(rbColorCache, rbColor);
  return rbColor;
}

static VALUE
Color_alloc(VALUE klass)
{
  return Data_Wrap_Struct(klass, 0, 0, (void*)0);
}

static VALUE
Color_initialize(VALUE self, VALUE rbRed, VALUE rbGreen, VALUE rbBlue, VALUE rbAlpha)
{
  int red   = NUM2INT(rbRed);
  int green = NUM2INT(rbGreen);
  int blue  = NUM2INT(rbBlue);
  int alpha = NUM2INT(rbAlpha);
  Pixel pixel = {
    .color = {
      .red   = red,
      .green = green,
      .blue  = blue,
      .alpha = alpha,
    }
  };
  // Only for Ruby 1.8?
  RDATA(self)->data = (void*)(VALUE)pixel.value;
  return Qnil;
}

static VALUE
Color_alpha(VALUE self)
{
  Color color;
  strb_GetColorFromRubyValue(&color, self);
  return INT2NUM(color.alpha);
}

static VALUE
Color_blue(VALUE self)
{
  Color color;
  strb_GetColorFromRubyValue(&color, self);
  return INT2NUM(color.blue);
}

static VALUE
Color_eq(VALUE self, VALUE rbOther)
{
  if (!rb_obj_is_kind_of(rbOther, rb_cColor))
    return Qfalse;
  Color color1, color2;
  strb_GetColorFromRubyValue(&color1, self);
  strb_GetColorFromRubyValue(&color2, rbOther);
  return (color1.red   == color2.red &&
          color1.green == color2.green &&
          color1.blue  == color2.blue &&
          color1.alpha == color2.alpha) ? Qtrue : Qfalse;
}

static VALUE
Color_green(VALUE self)
{
  Color color;
  strb_GetColorFromRubyValue(&color, self);
  return INT2NUM(color.green);
}

static VALUE
Color_hash(VALUE self)
{
  Color color;
  strb_GetColorFromRubyValue(&color, self);
  return INT2NUM(CalcColorHash(color.red, color.green, color.blue, color.alpha));
}

static VALUE
Color_red(VALUE self)
{
  Color color;
  strb_GetColorFromRubyValue(&color, self);
  return INT2NUM(color.red);
}

static VALUE
Color_to_s(VALUE self)
{
  Color color;
  strb_GetColorFromRubyValue(&color, self);
  char str[256];
  snprintf(str, sizeof(str),
           "#<StarRuby::Color alpha=%d, red=%d, green=%d, blue=%d>",
           color.alpha, color.red, color.green, color.blue);
  return rb_str_new2(str);
}

VALUE
strb_InitializeColor(VALUE rb_mStarRuby)
{
  rb_cColor = rb_define_class_under(rb_mStarRuby, "Color", rb_cObject);
  rb_define_singleton_method(rb_cColor, "new", Color_s_new, -1);
  rb_define_alloc_func(rb_cColor, Color_alloc);
  rb_define_private_method(rb_cColor, "initialize", Color_initialize, 4);
  rb_define_method(rb_cColor, "alpha", Color_alpha, 0);
  rb_define_method(rb_cColor, "blue",  Color_blue,  0);
  rb_define_method(rb_cColor, "==",    Color_eq,    1);
  rb_define_method(rb_cColor, "eql?",  Color_eq,    1);
  rb_define_method(rb_cColor, "green", Color_green, 0);
  rb_define_method(rb_cColor, "hash",  Color_hash,  0);
  rb_define_method(rb_cColor, "red",   Color_red,   0);
  rb_define_method(rb_cColor, "to_s",  Color_to_s,  0);

  int length = 128;
  rbColorCache = rb_iv_set(rb_cColor, "color_cache", rb_ary_new2(length));
  for (int i = 0; i < length; i++)
    rb_ary_push(rbColorCache, Qnil);

  return rb_cColor;
}