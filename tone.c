#include "starruby.h"

static VALUE Tone_alloc(VALUE klass)
{
  struct Tone* tone = ALLOC(struct Tone);
  return Data_Wrap_Struct(klass, 0, -1, tone);
}

static VALUE Tone_initialize(int argc, VALUE* argv, VALUE self)
{
  VALUE rbRed, rbGreen, rbBlue, rbSaturation;
  rb_scan_args(argc, argv, "31",
               &rbRed, &rbGreen, &rbBlue, &rbSaturation);
  struct Tone* tone;
  Data_Get_Struct(self, struct Tone, tone);
  tone->red   = NORMALIZE(NUM2INT(rbRed),   -255, 255);
  tone->green = NORMALIZE(NUM2INT(rbGreen), -255, 255);
  tone->blue  = NORMALIZE(NUM2INT(rbBlue),  -255, 255);
  tone->saturation = (rbSaturation != Qnil) ?
    NORMALIZE(NUM2INT(rbSaturation), 0, 255) : 255;
  return Qnil;
}

static VALUE Tone_blue(VALUE self)
{
  struct Tone* tone;
  Data_Get_Struct(self, struct Tone, tone);
  return INT2NUM(tone->blue);
}

static VALUE Tone_eq(VALUE self, VALUE rbOther)
{
  if (!rb_obj_is_kind_of(rbOther, rb_cTone))
    return Qfalse;

  struct Tone* tone1;
  Data_Get_Struct(self, struct Tone, tone1);

  struct Tone* tone2;
  Data_Get_Struct(rbOther, struct Tone, tone2);

  return (tone1->red        == tone2->red &&
          tone1->green      == tone2->green &&
          tone1->blue       == tone2->blue &&
          tone1->saturation == tone2->saturation) ? Qtrue : Qfalse;
}

static VALUE Tone_green(VALUE self)
{
  struct Tone* tone;
  Data_Get_Struct(self, struct Tone, tone);
  return INT2NUM(tone->green);
}

static VALUE Tone_hash(VALUE self)
{
  struct Tone* tone;
  Data_Get_Struct(self, struct Tone, tone);
  return INT2NUM(tone->red ^ tone->green ^ tone->blue ^ tone->saturation);
}

static VALUE Tone_saturation(VALUE self)
{
  struct Tone* tone;
  Data_Get_Struct(self, struct Tone, tone);
  return INT2NUM(tone->saturation);
}

static VALUE Tone_red(VALUE self)
{
  struct Tone* tone;
  Data_Get_Struct(self, struct Tone, tone);
  return INT2NUM(tone->red);
}

static VALUE Tone_to_s(VALUE self)
{
  struct Tone* tone;
  Data_Get_Struct(self, struct Tone, tone);

  char str[256];
  snprintf(str, sizeof(str),
           "#<StarRuby::Tone red=%d, green=%d, blue=%d, saturation=%d>",
           tone->red, tone->green, tone->blue, tone->saturation);
  return rb_str_new2(str);
}

void InitializeTone(void)
{
  rb_cTone = rb_define_class_under(rb_mStarRuby, "Tone", rb_cObject);
  rb_define_alloc_func(rb_cTone, Tone_alloc);
  rb_define_private_method(rb_cTone, "initialize", Tone_initialize, -1);
  rb_define_method(rb_cTone, "blue",       Tone_blue,       0);
  rb_define_method(rb_cTone, "==",         Tone_eq,         1);
  rb_define_method(rb_cTone, "eql?",       Tone_eq,         1);
  rb_define_method(rb_cTone, "green",      Tone_green,      0);
  rb_define_method(rb_cTone, "hash",       Tone_hash,       0);
  rb_define_method(rb_cTone, "saturation", Tone_saturation, 0);
  rb_define_method(rb_cTone, "red",        Tone_red,        0);
  rb_define_method(rb_cTone, "to_s",       Tone_to_s,       0);
}
