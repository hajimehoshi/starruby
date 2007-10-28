#include "starruby.h"

static VALUE symbol_game_pad;
static VALUE symbol_keyboard;
static VALUE symbol_mouse;
static VALUE rbMouseLocation;

static VALUE Input_mouse_location(VALUE self)
{
  return rbMouseLocation;
}

static VALUE Input_pressed_keys(int argc, VALUE* argv, VALUE self)
{
  VALUE rbDevice, rbOptions;
  rb_scan_args(argc, argv, "11", &rbDevice, &rbOptions);
  if (NIL_P(rbOptions))
    rbOptions = rb_hash_new();
  return Qnil;
}

void UpdateInput()
{
  int x, y;
  SDL_GetMouseState(&x, &y);
  rbMouseLocation = rb_assoc_new(INT2NUM(x), INT2NUM(y));
  OBJ_FREEZE(rbMouseLocation);
}

void InitializeSdlInput()
{
  SDL_JoystickEventState(SDL_ENABLE);
}

void FinalizeSdlInput()
{
}

void InitializeInput(void)
{
  rb_cInput = rb_define_class_under(rb_mStarRuby, "Input", rb_cObject);
  rb_define_method(rb_cInput, "mouse_location", Input_mouse_location, 0);
  rb_define_method(rb_cInput, "pressed_keys",   Input_pressed_keys,   0);
  rb_funcall(rb_singleton_class(rb_cInput), rb_intern("private"), 1,
             ID2SYM(rb_intern("new")));

  symbol_game_pad = ID2SYM(rb_intern("game_pad"));
  symbol_keyboard = ID2SYM(rb_intern("keyboard"));
  symbol_mouse    = ID2SYM(rb_intern("mouse"));
}
