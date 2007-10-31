#include "starruby.h"

#define ADD_KEY(currentKey, _name, _sdlKey) do {\
  KeyboardKey* key = ALLOC(KeyboardKey);\
  key->rbSymbol = ID2SYM(rb_intern(_name));\
  key->sdlKey   = _sdlKey;\
  key->state    = 0;\
  key->next     = NULL;\
  currentKey->next = key;\
  currentKey = key;\
} while (false)
#define STR2SYM(x) ID2SYM(rb_intern(x))

static int sdlJoystickCount;
static SDL_Joystick** sdlJoysticks;

typedef struct KeyboardKey {
  VALUE rbSymbol;
  SDLKey sdlKey;
  int state;
  struct KeyboardKey* next;
} KeyboardKey;

KeyboardKey* keyboardKeys;

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
  KeyboardKey* key = keyboardKeys;
  while (key) {
    KeyboardKey* nextKey = key->next;
    free(key);
    key = nextKey;
  }
  keyboardKeys = NULL;
  
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

  keyboardKeys = ALLOC(KeyboardKey);
  keyboardKeys->rbSymbol = Qundef; // dummy
  keyboardKeys->sdlKey   = 0;
  keyboardKeys->state    = 0;
  keyboardKeys->next     = NULL;
  
  KeyboardKey* currentKey = keyboardKeys;
  for (int i = 0; i < SDLK_z - SDLK_a + 1; i++)
    ADD_KEY(currentKey, ((char[]){'a' + i, '\0'}), SDLK_a + i);
  for (int i = 0; i <= 9; i++)
    ADD_KEY(currentKey, ((char[]){'d', '0' + i, '\0'}), SDLK_0 + i);
  for (int i = 0; i < 15; i++) {
    char name[4];
    snprintf(name, sizeof(name), "f%d", i + 1);
    ADD_KEY(currentKey, name, SDLK_F1 + i);
  }
  for (int i = 0; i <= 9; i++) {
    ADD_KEY(currentKey, ((char[]){'n','u','m','p','a','d', '0' + i, '\0'}),
            SDLK_KP0 + i);
  }
  char* names[] = {
    "add",
    "back",
    "capslock", "clear",
    "decimal", "delete", "divide", "down",
    "end", "enter", "escape",
    "help", "home",
    "insert",
    "lcontrolkey", "left", "lmenu", "lshiftkey", "lwin",
    "multiply",
    "numlock",
    "pagedown", "pageup",
    "rcontrolkey", "right", "rmenu", "rshiftkey", "rwin",
    "scroll", "space", "subtract",
    "tab",
    "up",
    NULL,
  };
  SDLKey sdlKeys[] = {
    SDLK_KP_PLUS,
    SDLK_BACKSPACE,
    SDLK_CAPSLOCK, SDLK_CLEAR,
    SDLK_PERIOD, SDLK_DELETE, SDLK_KP_DIVIDE, SDLK_DOWN,
    SDLK_END, SDLK_RETURN, SDLK_ESCAPE,
    SDLK_HELP, SDLK_HOME,
    SDLK_INSERT,
    SDLK_LCTRL, SDLK_LEFT, SDLK_LALT, SDLK_LSHIFT, SDLK_LSUPER,
    SDLK_KP_MULTIPLY, SDLK_NUMLOCK,
    SDLK_PAGEDOWN, SDLK_PAGEUP,
    SDLK_RCTRL, SDLK_RIGHT, SDLK_RALT, SDLK_RSHIFT, SDLK_RSUPER,
    SDLK_SCROLLOCK, SDLK_SPACE, SDLK_KP_MINUS,
    SDLK_TAB,
    SDLK_UP,
    -1,
  };
  char** name = names;
  for (SDLKey* sdlKey = sdlKeys;
       *name;
       name++, sdlKey++)
    ADD_KEY(currentKey, *name, *sdlKey);
}

#ifdef DEBUG
static KeyboardKey* searchKey(const char* name)
{
  VALUE rbNameSymbol = STR2SYM(name);
  KeyboardKey* key = keyboardKeys;
  while (key) {
    if (key->rbSymbol == rbNameSymbol)
      return key;
    key = key->next;
  }
  return NULL;
}


void TestInput(void)
{
  printf("Begin Test: Input\n");

  assert(SDLK_a == searchKey("a")->sdlKey);
  assert(SDLK_b == searchKey("b")->sdlKey);
  assert(SDLK_c == searchKey("c")->sdlKey);
  assert(SDLK_e == searchKey("e")->sdlKey);
  assert(SDLK_i == searchKey("i")->sdlKey);
  assert(SDLK_q == searchKey("q")->sdlKey);
  assert(SDLK_z == searchKey("z")->sdlKey);

  assert(SDLK_0 == searchKey("d0")->sdlKey);
  assert(SDLK_1 == searchKey("d1")->sdlKey);
  assert(SDLK_2 == searchKey("d2")->sdlKey);
  assert(SDLK_4 == searchKey("d4")->sdlKey);
  assert(SDLK_8 == searchKey("d8")->sdlKey);
  assert(SDLK_9 == searchKey("d9")->sdlKey);

  assert(SDLK_F1  == searchKey("f1")->sdlKey);
  assert(SDLK_F2  == searchKey("f2")->sdlKey);
  assert(SDLK_F4  == searchKey("f4")->sdlKey);
  assert(SDLK_F8  == searchKey("f8")->sdlKey);
  assert(SDLK_F15 == searchKey("f15")->sdlKey);

  assert(SDLK_KP0 == searchKey("numpad0")->sdlKey);
  assert(SDLK_KP1 == searchKey("numpad1")->sdlKey);
  assert(SDLK_KP2 == searchKey("numpad2")->sdlKey);
  assert(SDLK_KP4 == searchKey("numpad4")->sdlKey);
  assert(SDLK_KP8 == searchKey("numpad8")->sdlKey);
  assert(SDLK_KP9 == searchKey("numpad9")->sdlKey);

  assert(SDLK_DOWN  == searchKey("down")->sdlKey);
  assert(SDLK_LEFT  == searchKey("left")->sdlKey);
  assert(SDLK_RIGHT == searchKey("right")->sdlKey);
  assert(SDLK_UP    == searchKey("up")->sdlKey);

  for (KeyboardKey* key = keyboardKeys; key; key = key->next)
    assert(0 == key->state);

  printf("End Test: Input\n");
}
#endif