#include "starruby.h"

#define STR2SYM(x) ID2SYM(rb_intern(x))

static int sdlJoystickCount;
static SDL_Joystick** sdlJoysticks;

static VALUE symbol_game_pad;
static VALUE symbol_keyboard;
static VALUE symbol_mouse;

static VALUE Input_mouse_location(VALUE self)
{
  return rb_iv_get(rb_mInput, "mouse_location");
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
  Uint8 sdlMouseButtons = SDL_GetMouseState(&mouseLocationX, &mouseLocationY);
  VALUE rbMouseLocation = rb_assoc_new(INT2NUM(mouseLocationX),
                                       INT2NUM(mouseLocationY));
  OBJ_FREEZE(rbMouseLocation);

  if (sdlMouseButtons & SDL_BUTTON(SDL_BUTTON_LEFT))
    ;
  if (sdlMouseButtons & SDL_BUTTON(SDL_BUTTON_MIDDLE))
    ;
  if (sdlMouseButtons & SDL_BUTTON(SDL_BUTTON_RIGHT))
    ;
  
  rb_iv_set(rb_mInput, "mouse_location", rbMouseLocation);
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

  VALUE rbMap = rb_hash_new();
  rb_iv_set(rb_mInput, "keyboard_key_map", rbMap);
  
  for (int i = 0; i < SDLK_z - SDLK_a + 1; i++) {
    char keyName[] = {'a' + i, '\0'};
    rb_hash_aset(rbMap, ID2SYM(rb_intern(keyName)), INT2NUM(SDLK_a + i));
  }
  for (int i = 0; i <= 9; i++) {
    char keyName[] = {'d', '0' + i, '\0'};
    rb_hash_aset(rbMap, ID2SYM(rb_intern(keyName)), INT2NUM(SDLK_0 + i));
  }
  for (int i = 0; i < 15; i++) {
    char keyName[4];
    snprintf(keyName, sizeof(keyName), "f%d", i + 1);
    rb_hash_aset(rbMap, ID2SYM(rb_intern(keyName)), INT2NUM(SDLK_F1 + i));
  }
  for (int i = 0; i <= 9; i++) {
    char keyName[] = {'n', 'u', 'm', 'p', 'a', 'd', '0' + i, '\0'};
    rb_hash_aset(rbMap,ID2SYM(rb_intern(keyName)), INT2NUM(SDLK_KP0 + i));
  }
  rb_hash_aset(rbMap, STR2SYM("add"),         INT2NUM(SDLK_KP_PLUS));
  rb_hash_aset(rbMap, STR2SYM("back"),        INT2NUM(SDLK_BACKSPACE));
  rb_hash_aset(rbMap, STR2SYM("capslock"),    INT2NUM(SDLK_CAPSLOCK));
  rb_hash_aset(rbMap, STR2SYM("clear"),       INT2NUM(SDLK_CLEAR));
  rb_hash_aset(rbMap, STR2SYM("decimal"),     INT2NUM(SDLK_PERIOD));
  rb_hash_aset(rbMap, STR2SYM("delete"),      INT2NUM(SDLK_DELETE));
  rb_hash_aset(rbMap, STR2SYM("divide"),      INT2NUM(SDLK_KP_DIVIDE));
  rb_hash_aset(rbMap, STR2SYM("down"),        INT2NUM(SDLK_DOWN));
  rb_hash_aset(rbMap, STR2SYM("end"),         INT2NUM(SDLK_END));
  rb_hash_aset(rbMap, STR2SYM("enter"),       INT2NUM(SDLK_RETURN));
  rb_hash_aset(rbMap, STR2SYM("escape"),      INT2NUM(SDLK_ESCAPE));
  rb_hash_aset(rbMap, STR2SYM("help"),        INT2NUM(SDLK_HELP));
  rb_hash_aset(rbMap, STR2SYM("home"),        INT2NUM(SDLK_HOME));
  rb_hash_aset(rbMap, STR2SYM("insert"),      INT2NUM(SDLK_INSERT));
  rb_hash_aset(rbMap, STR2SYM("lcontrolkey"), INT2NUM(SDLK_LCTRL));
  rb_hash_aset(rbMap, STR2SYM("left"),        INT2NUM(SDLK_LEFT));
  rb_hash_aset(rbMap, STR2SYM("lmenu"),       INT2NUM(SDLK_LALT));
  rb_hash_aset(rbMap, STR2SYM("lshiftkey"),   INT2NUM(SDLK_LSHIFT));
  rb_hash_aset(rbMap, STR2SYM("lwin"),        INT2NUM(SDLK_LSUPER));
  rb_hash_aset(rbMap, STR2SYM("multiply"),    INT2NUM(SDLK_KP_MULTIPLY));
  rb_hash_aset(rbMap, STR2SYM("numlock"),     INT2NUM(SDLK_NUMLOCK));
  rb_hash_aset(rbMap, STR2SYM("pagedown"),    INT2NUM(SDLK_PAGEDOWN));
  rb_hash_aset(rbMap, STR2SYM("pageup"),      INT2NUM(SDLK_PAGEUP));
  rb_hash_aset(rbMap, STR2SYM("rcontrolkey"), INT2NUM(SDLK_RCTRL));
  rb_hash_aset(rbMap, STR2SYM("right"),       INT2NUM(SDLK_RIGHT));
  rb_hash_aset(rbMap, STR2SYM("rmenu"),       INT2NUM(SDLK_RALT));
  rb_hash_aset(rbMap, STR2SYM("rshiftkey"),   INT2NUM(SDLK_RSHIFT));
  rb_hash_aset(rbMap, STR2SYM("rwin"),        INT2NUM(SDLK_RSUPER));
  rb_hash_aset(rbMap, STR2SYM("scroll"),      INT2NUM(SDLK_SCROLLOCK));
  rb_hash_aset(rbMap, STR2SYM("space"),       INT2NUM(SDLK_SPACE));
  rb_hash_aset(rbMap, STR2SYM("subtract"),    INT2NUM(SDLK_KP_MINUS));
  rb_hash_aset(rbMap, STR2SYM("tab"),         INT2NUM(SDLK_TAB));
  rb_hash_aset(rbMap, STR2SYM("up"),          INT2NUM(SDLK_UP));
}

#ifdef DEBUG
void TestInput(void)
{
  printf("Begin Test: Input\n");
  
  VALUE rbMap = rb_iv_get(rb_mInput, "keyboard_key_map");
  assert(SDLK_a   == NUM2INT(rb_hash_aref(rbMap, STR2SYM("a"))));
  assert(SDLK_b   == NUM2INT(rb_hash_aref(rbMap, STR2SYM("b"))));
  assert(SDLK_c   == NUM2INT(rb_hash_aref(rbMap, STR2SYM("c"))));
  assert(SDLK_e   == NUM2INT(rb_hash_aref(rbMap, STR2SYM("e"))));
  assert(SDLK_i   == NUM2INT(rb_hash_aref(rbMap, STR2SYM("i"))));
  assert(SDLK_q   == NUM2INT(rb_hash_aref(rbMap, STR2SYM("q"))));
  assert(SDLK_z   == NUM2INT(rb_hash_aref(rbMap, STR2SYM("z"))));
  assert(SDLK_0   == NUM2INT(rb_hash_aref(rbMap, STR2SYM("d0"))));
  assert(SDLK_1   == NUM2INT(rb_hash_aref(rbMap, STR2SYM("d1"))));
  assert(SDLK_2   == NUM2INT(rb_hash_aref(rbMap, STR2SYM("d2"))));
  assert(SDLK_4   == NUM2INT(rb_hash_aref(rbMap, STR2SYM("d4"))));
  assert(SDLK_8   == NUM2INT(rb_hash_aref(rbMap, STR2SYM("d8"))));
  assert(SDLK_9   == NUM2INT(rb_hash_aref(rbMap, STR2SYM("d9"))));
  assert(SDLK_F1  == NUM2INT(rb_hash_aref(rbMap, STR2SYM("f1"))));
  assert(SDLK_F2  == NUM2INT(rb_hash_aref(rbMap, STR2SYM("f2"))));
  assert(SDLK_F4  == NUM2INT(rb_hash_aref(rbMap, STR2SYM("f4"))));
  assert(SDLK_F8  == NUM2INT(rb_hash_aref(rbMap, STR2SYM("f8"))));
  assert(SDLK_F15 == NUM2INT(rb_hash_aref(rbMap, STR2SYM("f15"))));
  assert(SDLK_KP0 == NUM2INT(rb_hash_aref(rbMap, STR2SYM("numpad0"))));
  assert(SDLK_KP1 == NUM2INT(rb_hash_aref(rbMap, STR2SYM("numpad1"))));
  assert(SDLK_KP2 == NUM2INT(rb_hash_aref(rbMap, STR2SYM("numpad2"))));
  assert(SDLK_KP4 == NUM2INT(rb_hash_aref(rbMap, STR2SYM("numpad4"))));
  assert(SDLK_KP8 == NUM2INT(rb_hash_aref(rbMap, STR2SYM("numpad8"))));
  assert(SDLK_KP9 == NUM2INT(rb_hash_aref(rbMap, STR2SYM("numpad9"))));

  printf("End Test: Input\n");
}
#endif
