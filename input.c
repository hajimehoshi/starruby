#include "starruby.h"

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

  VALUE rbKeyboardKeyMap = rb_hash_new();
  rb_iv_set(rb_mInput, "keyboard_key_map", rbKeyboardKeyMap);
  
  char alphabets[2] = {'\0', '\0'};
  for (int i = 0; i < SDLK_z - SDLK_a + 1; i++) {
    alphabets[0] = 'a' + i;
    rb_hash_aset(rbKeyboardKeyMap,
                 ID2SYM(rb_intern(&(alphabets[0]))), INT2NUM(SDLK_a + i));
  }
  for (int i = 0; i <= 9; i++) {
    char keyName[3];
    snprintf(keyName, sizeof(keyName), "d%d", i);
    rb_hash_aset(rbKeyboardKeyMap,
                 ID2SYM(rb_intern(keyName)), INT2NUM(SDLK_0 + i));
  }
  for (int i = 0; i < 15; i++) {
    char keyName[4];
    snprintf(keyName, sizeof(keyName), "f%d", i + 1);
    rb_hash_aset(rbKeyboardKeyMap,
                 ID2SYM(rb_intern(keyName)), INT2NUM(SDLK_F1 + i));
  }
  for (int i = 0; i <= 9; i++) {
    char keyName[8];
    snprintf(keyName, sizeof(keyName), "numpad%d", i);
    rb_hash_aset(rbKeyboardKeyMap,
                 ID2SYM(rb_intern(keyName)), INT2NUM(SDLK_KP0 + i));
  }
}

#ifdef DEBUG
void TestInput(void)
{
  printf("Begin Test: Input\n");
  
  VALUE rbMap = rb_iv_get(rb_mInput, "keyboard_key_map");
  assert(SDLK_a   == NUM2INT(rb_hash_aref(rbMap, ID2SYM(rb_intern("a")))));
  assert(SDLK_b   == NUM2INT(rb_hash_aref(rbMap, ID2SYM(rb_intern("b")))));
  assert(SDLK_c   == NUM2INT(rb_hash_aref(rbMap, ID2SYM(rb_intern("c")))));
  assert(SDLK_e   == NUM2INT(rb_hash_aref(rbMap, ID2SYM(rb_intern("e")))));
  assert(SDLK_i   == NUM2INT(rb_hash_aref(rbMap, ID2SYM(rb_intern("i")))));
  assert(SDLK_q   == NUM2INT(rb_hash_aref(rbMap, ID2SYM(rb_intern("q")))));
  assert(SDLK_z   == NUM2INT(rb_hash_aref(rbMap, ID2SYM(rb_intern("z")))));
  assert(SDLK_0   == NUM2INT(rb_hash_aref(rbMap, ID2SYM(rb_intern("d0")))));
  assert(SDLK_1   == NUM2INT(rb_hash_aref(rbMap, ID2SYM(rb_intern("d1")))));
  assert(SDLK_2   == NUM2INT(rb_hash_aref(rbMap, ID2SYM(rb_intern("d2")))));
  assert(SDLK_4   == NUM2INT(rb_hash_aref(rbMap, ID2SYM(rb_intern("d4")))));
  assert(SDLK_8   == NUM2INT(rb_hash_aref(rbMap, ID2SYM(rb_intern("d8")))));
  assert(SDLK_9   == NUM2INT(rb_hash_aref(rbMap, ID2SYM(rb_intern("d9")))));
  assert(SDLK_F1  == NUM2INT(rb_hash_aref(rbMap, ID2SYM(rb_intern("f1")))));
  assert(SDLK_F2  == NUM2INT(rb_hash_aref(rbMap, ID2SYM(rb_intern("f2")))));
  assert(SDLK_F4  == NUM2INT(rb_hash_aref(rbMap, ID2SYM(rb_intern("f4")))));
  assert(SDLK_F8  == NUM2INT(rb_hash_aref(rbMap, ID2SYM(rb_intern("f8")))));
  assert(SDLK_F15 == NUM2INT(rb_hash_aref(rbMap, ID2SYM(rb_intern("f15")))));
  assert(SDLK_KP0 == NUM2INT(rb_hash_aref(rbMap, ID2SYM(rb_intern("numpad0")))));
  assert(SDLK_KP1 == NUM2INT(rb_hash_aref(rbMap, ID2SYM(rb_intern("numpad1")))));
  assert(SDLK_KP2 == NUM2INT(rb_hash_aref(rbMap, ID2SYM(rb_intern("numpad2")))));
  assert(SDLK_KP4 == NUM2INT(rb_hash_aref(rbMap, ID2SYM(rb_intern("numpad4")))));
  assert(SDLK_KP8 == NUM2INT(rb_hash_aref(rbMap, ID2SYM(rb_intern("numpad8")))));
  assert(SDLK_KP9 == NUM2INT(rb_hash_aref(rbMap, ID2SYM(rb_intern("numpad9")))));

  printf("End Test: Input\n");
}
#endif
