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
  rb_cInput = rb_define_class_under(rb_mStarRuby, "Input", rb_cObject);
  rb_define_method(rb_cInput, "mouse_location", Input_mouse_location, 0);
  rb_funcall(rb_singleton_class(rb_cInput), rb_intern("private"), 1,
             ID2SYM(rb_intern("new")));
}
