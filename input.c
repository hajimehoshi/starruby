#include "starruby.h"

static int sdlJoystickCount;
static SDL_Joystick** sdlJoysticks;

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
  SDL_JoystickUpdate();

  // Keyboard

  // Game Pad

  // Mouse
  int mouseLocationX, mouseLocationY;
  SDL_GetMouseState(&mouseLocationX, &mouseLocationY);
  rbMouseLocation = rb_assoc_new(INT2NUM(mouseLocationX),
                                 INT2NUM(mouseLocationY));
  OBJ_FREEZE(rbMouseLocation);
}

void InitializeSdlInput()
{
  SDL_JoystickEventState(SDL_ENABLE);
  sdlJoystickCount = SDL_NumJoysticks();
  sdlJoysticks = ALLOC_N(SDL_Joystick*, sdlJoystickCount);
  for (int i = 0; i < sdlJoystickCount; i++) {
    sdlJoysticks[i] = SDL_JoystickOpen(i);
  }
}

void FinalizeSdlInput()
{
  for (int i = 0; i < sdlJoystickCount; i++) {
    if (SDL_JoystickOpened(i))
      SDL_JoystickClose(sdlJoysticks[i]);
    sdlJoysticks[i] = NULL;
  }
  free(sdlJoysticks);
  sdlJoysticks = NULL;
}

void InitializeInput(void)
{
  rb_mInput = rb_define_module_under(rb_mStarRuby, "Input");
  rb_define_singleton_method(rb_mInput, "mouse_location",
                             Input_mouse_location, 0);
  rb_define_singleton_method(rb_mInput, "pressed_keys",
                             Input_pressed_keys,   0);
  
  symbol_game_pad = ID2SYM(rb_intern("game_pad"));
  symbol_keyboard = ID2SYM(rb_intern("keyboard"));
  symbol_mouse    = ID2SYM(rb_intern("mouse"));
}
