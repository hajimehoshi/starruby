#include "starruby.h"
#include "starruby_private.h"

static volatile VALUE rb_cColor;
static volatile VALUE rbColorCache;

VALUE
strb_GetColorClass(void)
{
  return rb_cColor;
}

static void Color_free(Color*);
inline void
strb_GetColorFromRubyValue(Color* color, VALUE rbColor)
{
  Check_Type(rbColor, T_DATA);
  if (RDATA(rbColor)->dfree != (RUBY_DATA_FUNC)Color_free) {
    rb_raise(rb_eTypeError, "wrong argument type %s (expected StarRuby::Color)",
             rb_obj_classname(rbColor));
  }
  const Pixel p = (Pixel){
    .value = (uint32_t)(VALUE)DATA_PTR(rbColor)
  };
  *color = p.color;
}

static VALUE
Color_s_new(int argc, VALUE* argv, VALUE self)
{
  volatile VALUE rbRed, rbGreen, rbBlue, rbAlpha;
  rb_scan_args(argc, argv, "31",
               &rbRed, &rbGreen, &rbBlue, &rbAlpha);
  const int red   = NUM2INT(rbRed);
  const int green = NUM2INT(rbGreen);
  const int blue  = NUM2INT(rbBlue);
  const int alpha = NIL_P(rbAlpha) ? 255 : NUM2INT(rbAlpha);
  if (red < 0 || 256 <= red || green < 0 || 256 <= green ||
      blue < 0 || 256 <= blue || alpha < 0 || 256 <= alpha) {
    rb_raise(rb_eArgError, "invalid color value: (r:%d, g:%d, b:%d, a:%d)",
             red, green, blue, alpha);
  }
  const VALUE* rbColorCacheP = RARRAY_PTR(rbColorCache);
  const int length = RARRAY_LEN(rbColorCache);
  for (int i = 0; i < length; i++, rbColorCacheP++) {
    volatile VALUE rbColor = *rbColorCacheP;
    if (NIL_P(rbColor)) {
      break;
    }
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
    rbRed, rbGreen, rbBlue, NIL_P(rbAlpha) ? INT2FIX(255) : rbAlpha,
  };
  VALUE rbColor = rb_class_new_instance(sizeof(args) / sizeof(VALUE), args, self);
  rb_ary_pop(rbColorCache);
  rb_ary_unshift(rbColorCache, rbColor);
  return rbColor;
}

static void
Color_free(Color* color)
{
}

static VALUE
Color_alloc(VALUE klass)
{
  return Data_Wrap_Struct(klass, 0, Color_free, (void*)0);
}

static VALUE
Color_initialize(VALUE self, VALUE rbRed, VALUE rbGreen, VALUE rbBlue, VALUE rbAlpha)
{
  const int red   = NUM2INT(rbRed);
  const int green = NUM2INT(rbGreen);
  const int blue  = NUM2INT(rbBlue);
  const int alpha = NUM2INT(rbAlpha);
  const Pixel pixel = {
    .color = {
      .red   = red,
      .green = green,
      .blue  = blue,
      .alpha = alpha,
    }
  };
  // Valid at Ruby 1.9?
  RDATA(self)->data = (void*)(VALUE)pixel.value;
  return Qnil;
}

static VALUE
Color_alpha(VALUE self)
{
  Color color;
  strb_GetColorFromRubyValue(&color, self);
  return INT2FIX(color.alpha);
}

static VALUE
Color_blue(VALUE self)
{
  Color color;
  strb_GetColorFromRubyValue(&color, self);
  return INT2FIX(color.blue);
}

static VALUE
Color_equal(VALUE self, VALUE rbOther)
{
  if (self == rbOther) {
    return Qtrue;
  }
  if (!rb_obj_is_kind_of(rbOther, rb_cColor)) {
    return Qfalse;
  }
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
  return INT2FIX(color.green);
}

static VALUE
Color_hash(VALUE self)
{
  Color color;
  strb_GetColorFromRubyValue(&color, self);
#if POSFIXABLE(0xffffffff)
  const uint32_t hash = (color.alpha << 24) |
    (color.red << 16) |
    (color.green << 8) |
    color.blue;
#else
  const uint32_t hash = ((color.alpha >> 6) << 24) |
    ((color.red ^ ((color.alpha >> 4) & 0x3)) << 16) |
    ((color.green ^ ((color.alpha >> 2) & 0x3)) << 8) |
    (color.blue ^ (color.alpha & 0x3));
#endif
  return INT2FIX(hash);
}

static VALUE
Color_red(VALUE self)
{
  Color color;
  strb_GetColorFromRubyValue(&color, self);
  return INT2FIX(color.red);
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
  rb_define_method(rb_cColor, "==",    Color_equal, 1);
  rb_define_method(rb_cColor, "eql?",  Color_equal, 1);
  rb_define_method(rb_cColor, "green", Color_green, 0);
  rb_define_method(rb_cColor, "hash",  Color_hash,  0);
  rb_define_method(rb_cColor, "red",   Color_red,   0);
  rb_define_method(rb_cColor, "to_s",  Color_to_s,  0);

  const int length = 128;
  rbColorCache = rb_iv_set(rb_cColor, "color_cache", rb_ary_new2(length));
  for (int i = 0; i < length; i++) {
    rb_ary_push(rbColorCache, Qnil);
  }
  return rb_cColor;
}
