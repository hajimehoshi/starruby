#include "starruby.h"

static VALUE Input_mouse_location(VALUE self)
{
  int x, y;
  SDL_GetMouseState(&x, &y);
  VALUE rbLocation = rb_assoc_new(INT2NUM(x), INT2NUM(y));
  OBJ_FREEZE(rbLocation);
  return rbLocation;
}

static VALUE Input_pressed_keys(int argc, VALUE* argv, VALUE self)
{
  return Qnil;
}

void InitializeInput(void)
{
  VALUE rb_mInput = rb_define_module_under(rb_mStarRuby, "Input");
  rb_define_singleton_method(rb_mInput, "mouse_location",
                             Input_mouse_location, 0);
  rb_define_singleton_method(rb_mInput, "pressed_keys",
                             Input_pressed_keys, -1);
}
