#include "starruby_private.h"

typedef struct KeyboardKey {
  VALUE rbSymbol;
  SDLKey sdlKey;
  int state;
  struct KeyboardKey* next;
} KeyboardKey;
static KeyboardKey* keyboardKeys;

static int gamepadCount;

typedef struct {
  SDL_Joystick* sdlJoystick;
  int downState;
  int leftState;
  int rightState;
  int upState;
  int buttonCount;
  int* buttonStates;
} Gamepad;
static Gamepad* gamepads;

typedef struct {
  int leftState;
  int middleState;
  int rightState;
} Mouse;
static Mouse* mouse;

static volatile VALUE rb_mInput = Qundef;

static volatile VALUE symbol_delay         = Qundef;
static volatile VALUE symbol_device_number = Qundef;
static volatile VALUE symbol_down          = Qundef;
static volatile VALUE symbol_duration      = Qundef;
static volatile VALUE symbol_gamepad       = Qundef;
static volatile VALUE symbol_interval      = Qundef;
static volatile VALUE symbol_keyboard      = Qundef;
static volatile VALUE symbol_left          = Qundef;
static volatile VALUE symbol_middle        = Qundef;
static volatile VALUE symbol_mouse         = Qundef;
static volatile VALUE symbol_right         = Qundef;
static volatile VALUE symbol_up            = Qundef;

static VALUE
Input_gamepad_count(VALUE self)
{
  return INT2FIX(gamepadCount);
}

static VALUE
Input_mouse_location(VALUE self)
{
  return rb_iv_get(self, "mouse_location");
}

static VALUE
Input_mouse_location_eq(VALUE self, VALUE rbValue)
{
  Check_Type(rbValue, T_ARRAY);
  if (RARRAY_LEN(rbValue) != 2) {
    rb_raise(rb_eArgError, "array size should be 2, %ld given", RARRAY_LEN(rbValue));
  }
  const int windowScale = strb_GetWindowScale();
  // TODO: Fix it for fullscreen
  SDL_WarpMouse(NUM2INT(RARRAY_PTR(rbValue)[0]) * windowScale,
                NUM2INT(RARRAY_PTR(rbValue)[1]) * windowScale);
  int mouseLocationX, mouseLocationY;
  SDL_GetMouseState(&mouseLocationX, &mouseLocationY);
  int screenWidth = 0, screenHeight = 0;
  strb_GetScreenSize(&screenWidth, &screenHeight);
  int realScreenWidth = 0, realScreenHeight = 0;
  strb_GetRealScreenSize(&realScreenWidth, &realScreenHeight);
  mouseLocationX -= (realScreenWidth  - screenWidth  * windowScale) / 2;
  mouseLocationY -= (realScreenHeight - screenHeight * windowScale) / 2;
  volatile VALUE rbMouseLocation =
    rb_assoc_new(INT2NUM(mouseLocationX / windowScale),
                 INT2NUM(mouseLocationY / windowScale));
  OBJ_FREEZE(rbMouseLocation);
  rb_iv_set(self, "mouse_location", rbMouseLocation);  
  return rbValue;
}

static bool
IsPressed(const int status, const int duration, const int delay, const int interval)
{
  /*
   * on:  ------------         -            -             ...
   * off:             --------- ------------ ------------ ...
   *      <-duration-><-delay-> <-interval-> <-interval-> ...
   */
  if (status <= 0 || duration == 0) {
    return false;
  }
  if (duration < 0) {
    return true;
  }
  if (status <= duration) {
    return true;
  }
  if (delay < 0) {
    return false;
  }
  if (status <= duration + delay) {
    return false;
  }
  if (0 <= interval) {
    return (status - (duration + delay + 1)) % (interval + 1) == 0;
  }
  return false;
}

static VALUE
Input_keys(int argc, VALUE* argv, VALUE self)
{
  volatile VALUE rbDevice, rbOptions;
  rb_scan_args(argc, argv, "11", &rbDevice, &rbOptions);
  Check_Type(rbDevice, T_SYMBOL);
  if (NIL_P(rbOptions)) {
    rbOptions = rb_hash_new();
  }
  volatile VALUE rbResult = rb_ary_new();

  int deviceNumber = 0;
  int duration     = -1;
  int delay        = -1;
  int interval     = 0;

  volatile VALUE val;
  Check_Type(rbOptions, T_HASH);
  if (!NIL_P(val = rb_hash_aref(rbOptions, symbol_device_number))) {
    deviceNumber = NUM2INT(val);
  }
  if (!NIL_P(val = rb_hash_aref(rbOptions, symbol_duration))) {
    duration = NUM2INT(val);
  }
  if (!NIL_P(val = rb_hash_aref(rbOptions, symbol_delay))) {
    delay = NUM2INT(val);
  }
  if (!NIL_P(val = rb_hash_aref(rbOptions, symbol_interval))) {
    interval = NUM2INT(val);
  }
  if (rbDevice == symbol_keyboard) {
    KeyboardKey* key = keyboardKeys;
    while (key) {
      if (IsPressed(key->state, duration, delay, interval)) {
        rb_ary_push(rbResult, key->rbSymbol);
      }
      key = key->next;
    }
  } else if (rbDevice == symbol_gamepad) {
    if (0 <= deviceNumber && deviceNumber < gamepadCount) {
      Gamepad* gamepad = &(gamepads[deviceNumber]);
      if (IsPressed(gamepad->downState, duration, delay, interval)) {
        rb_ary_push(rbResult, symbol_down);
      }
      if (IsPressed(gamepad->leftState, duration, delay, interval)) {
        rb_ary_push(rbResult, symbol_left);
      }
      if (IsPressed(gamepad->rightState, duration, delay, interval)) {
        rb_ary_push(rbResult, symbol_right);
      }
      if (IsPressed(gamepad->upState, duration, delay, interval)) {
        rb_ary_push(rbResult, symbol_up);
      }
      for (int i = 0; i < gamepad->buttonCount; i++) {
        if (IsPressed(gamepad->buttonStates[i], duration, delay, interval)) {
          rb_ary_push(rbResult, INT2FIX(i + 1));
        }
      }
    }
  } else if (rbDevice == symbol_mouse) {
    if (IsPressed(mouse->leftState, duration, delay, interval)) {
      rb_ary_push(rbResult, symbol_left);
    }
    if (IsPressed(mouse->middleState, duration, delay, interval)) {
      rb_ary_push(rbResult, symbol_middle);
    }
    if (IsPressed(mouse->rightState, duration, delay, interval)) {
      rb_ary_push(rbResult, symbol_right);
    }
  } else {
    volatile VALUE rbDeviceInspect =
      rb_funcall(rbDevice, rb_intern("inspect"), 0);
    rb_raise(rb_eArgError, "invalid device: %s", StringValueCStr(rbDeviceInspect));
  }

  OBJ_FREEZE(rbResult);
  return rbResult;
}

#define ADD_KEY(currentKey, _name, _sdlKey)   \
  do {                                        \
    KeyboardKey* key = ALLOC(KeyboardKey);    \
    key->rbSymbol = ID2SYM(rb_intern(_name)); \
    key->sdlKey   = _sdlKey;                  \
    key->state    = 0;                        \
    key->next     = NULL;                     \
    currentKey->next = key;                   \
    currentKey = key;                         \
  } while (false)

void
strb_UpdateInput(void)
{
  SDL_PumpEvents();
  SDL_JoystickUpdate();

  const Uint8* sdlKeyState = SDL_GetKeyState(NULL);
  KeyboardKey* key = keyboardKeys;
  while (key) {
    if (sdlKeyState[key->sdlKey]) {
      key->state++;
    } else {
      key->state = 0;
    }
    key = key->next;
  }

  for (int i = 0; i < gamepadCount; i++) {
    Gamepad* gamepad = &(gamepads[i]);
    if (SDL_JoystickGetAxis(gamepad->sdlJoystick, 1) > 3200) {
      gamepad->downState++;
    } else {
      gamepad->downState = 0;
    }
    if (SDL_JoystickGetAxis(gamepad->sdlJoystick, 0) < -3200) {
      gamepad->leftState++;
    } else {
      gamepad->leftState = 0;
    }
    if (SDL_JoystickGetAxis(gamepad->sdlJoystick, 0) > 3200) {
      gamepad->rightState++;
    } else {
      gamepad->rightState = 0;
    }
    if (SDL_JoystickGetAxis(gamepad->sdlJoystick, 1) < -3200) {
      gamepad->upState++;
    } else {
      gamepad->upState = 0;
    }
    for (int j = 0; j < gamepad->buttonCount; j++) {
      if (SDL_JoystickGetButton(gamepad->sdlJoystick, j) == SDL_PRESSED) {
        gamepad->buttonStates[j]++;
      } else {
        gamepad->buttonStates[j] = 0;
      }
    }
  }

  const int windowScale = strb_GetWindowScale();
  int mouseLocationX, mouseLocationY;
  const Uint8 sdlMouseButtons =
    SDL_GetMouseState(&mouseLocationX, &mouseLocationY);
  int screenWidth = 0, screenHeight = 0;
  strb_GetScreenSize(&screenWidth, &screenHeight);
  int realScreenWidth = 0, realScreenHeight = 0;
  strb_GetRealScreenSize(&realScreenWidth, &realScreenHeight);
  mouseLocationX -= (realScreenWidth  - screenWidth  * windowScale) / 2;
  mouseLocationY -= (realScreenHeight - screenHeight * windowScale) / 2;
  volatile VALUE rbMouseLocation =
    rb_assoc_new(INT2NUM(mouseLocationX / windowScale),
                 INT2NUM(mouseLocationY / windowScale));
  OBJ_FREEZE(rbMouseLocation);
  rb_iv_set(rb_mInput, "mouse_location", rbMouseLocation);

  if (sdlMouseButtons & SDL_BUTTON(SDL_BUTTON_LEFT)) {
    mouse->leftState++;
  } else {
    mouse->leftState = 0;
  }
  if (sdlMouseButtons & SDL_BUTTON(SDL_BUTTON_MIDDLE)) {
    mouse->middleState++;
  } else {
    mouse->middleState = 0;
  }
  if (sdlMouseButtons & SDL_BUTTON(SDL_BUTTON_RIGHT)) {
    mouse->rightState++;
  } else {
    mouse->rightState = 0;
  }
}

void
strb_InitializeSdlInput()
{
  keyboardKeys = ALLOC(KeyboardKey);
  keyboardKeys->rbSymbol = Qundef;
  keyboardKeys->sdlKey   = 0;
  keyboardKeys->state    = 0;
  keyboardKeys->next     = NULL;
  
  KeyboardKey* currentKey = keyboardKeys;
  for (int i = 0; i < SDLK_z - SDLK_a + 1; i++) {
    ADD_KEY(currentKey, ((char[]){'a' + i, '\0'}), SDLK_a + i);
  }
  for (int i = 0; i <= 9; i++) {
    ADD_KEY(currentKey, ((char[]){'d', '0' + i, '\0'}), SDLK_0 + i);
  }
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
    "back", "backslash", "backquotes",
    "capslock", "clear", "closebrackets", "comma",
    "decimal", "delete", "divide", "down",
    "end", "enter", "escape", "equals",
    "help", "home",
    "insert",
    "lcontrolkey", "left", "lmenu", "lshiftkey",
    "minus", "multiply",
    "numlock",
    "openbrackets",
    "pagedown", "pageup", "period",
    "quotes",
    "rcontrolkey", "right", "rmenu", "rshiftkey",
    "scroll", "semicolon", "separator", "slash", "space", "subtract",
    "tab",
    "up",
    NULL,
  };
  SDLKey sdlKeys[] = {
    SDLK_KP_PLUS,
    SDLK_BACKSPACE, SDLK_BACKSLASH, SDLK_BACKQUOTE,
    SDLK_CAPSLOCK, SDLK_CLEAR, SDLK_RIGHTBRACKET, SDLK_COMMA,
    SDLK_KP_PERIOD, SDLK_DELETE, SDLK_KP_DIVIDE, SDLK_DOWN,
    SDLK_END, SDLK_RETURN, SDLK_ESCAPE, SDLK_EQUALS,
    SDLK_HELP, SDLK_HOME,
    SDLK_INSERT,
    SDLK_LCTRL, SDLK_LEFT, SDLK_LALT, SDLK_LSHIFT,
    SDLK_MINUS, SDLK_KP_MULTIPLY,
    SDLK_NUMLOCK,
    SDLK_LEFTBRACKET,
    SDLK_PAGEDOWN, SDLK_PAGEUP, SDLK_PERIOD,
    SDLK_QUOTE,
    SDLK_RCTRL, SDLK_RIGHT, SDLK_RALT, SDLK_RSHIFT,
    SDLK_SCROLLOCK, SDLK_SEMICOLON, SDLK_KP_ENTER, SDLK_SLASH, SDLK_SPACE, SDLK_KP_MINUS,
    SDLK_TAB,
    SDLK_UP,
    -1,
  };
  SDLKey* sdlKey;
  char** name;
  for (sdlKey = sdlKeys, name = names;
       *name;
       name++, sdlKey++) {
    ADD_KEY(currentKey, *name, *sdlKey);
  }
  SDL_JoystickEventState(SDL_ENABLE);
  gamepadCount = SDL_NumJoysticks();
  gamepads = ALLOC_N(Gamepad, gamepadCount);
  MEMZERO(gamepads, Gamepad, gamepadCount);
  for (int i = 0; i < gamepadCount; i++) {
    SDL_Joystick* ptr = gamepads[i].sdlJoystick = SDL_JoystickOpen(i);
    if (ptr == NULL) {
      rb_raise_sdl_error();
    }
    const int buttonCount = gamepads[i].buttonCount = SDL_JoystickNumButtons(ptr);
    gamepads[i].buttonStates = ALLOC_N(int, buttonCount);
    MEMZERO(gamepads[i].buttonStates, int, buttonCount);
  }

  mouse = ALLOC(Mouse);
  MEMZERO(mouse, Mouse, 1);
}

VALUE
strb_InitializeInput(VALUE rb_mStarRuby)
{
  rb_mInput = rb_define_module_under(rb_mStarRuby, "Input");
  rb_define_module_function(rb_mInput, "gamepad_count",
                            Input_gamepad_count, 0);
  rb_define_module_function(rb_mInput, "mouse_location",
                            Input_mouse_location, 0);
  rb_define_module_function(rb_mInput, "mouse_location=",
                            Input_mouse_location_eq, 1);
  rb_define_module_function(rb_mInput, "keys",   Input_keys, -1);

  symbol_delay         = ID2SYM(rb_intern("delay"));
  symbol_device_number = ID2SYM(rb_intern("device_number"));
  symbol_down          = ID2SYM(rb_intern("down"));
  symbol_duration      = ID2SYM(rb_intern("duration"));
  symbol_gamepad       = ID2SYM(rb_intern("gamepad"));
  symbol_interval      = ID2SYM(rb_intern("interval"));
  symbol_keyboard      = ID2SYM(rb_intern("keyboard"));
  symbol_left          = ID2SYM(rb_intern("left"));
  symbol_middle        = ID2SYM(rb_intern("middle"));
  symbol_mouse         = ID2SYM(rb_intern("mouse"));
  symbol_right         = ID2SYM(rb_intern("right"));
  symbol_up            = ID2SYM(rb_intern("up"));

  volatile VALUE rbMouseLocation = rb_assoc_new(INT2FIX(0), INT2FIX(0));
  OBJ_FREEZE(rbMouseLocation);
  rb_iv_set(rb_mInput, "mouse_location", rbMouseLocation);

  return rb_mInput;
}

void
strb_FinalizeInput(void)
{
  free(mouse);
  mouse = NULL;

  for (int i = 0; i < gamepadCount; i++) {
    Gamepad* gamepad = &(gamepads[i]);
    if (SDL_JoystickOpened(i)) {
      SDL_JoystickClose(gamepad->sdlJoystick);
    }
    gamepad->sdlJoystick = NULL;
    free(gamepad->buttonStates);
    gamepad->buttonStates = NULL;
  }
  free(gamepads);
  gamepads = NULL;
  
  KeyboardKey* key = keyboardKeys;
  while (key) {
    KeyboardKey* nextKey = key->next;
    free(key);
    key = nextKey;
  }
  keyboardKeys = NULL;
}

#ifdef DEBUG
static KeyboardKey*
searchKey(const char* name)
{
  volatile VALUE rbNameSymbol = ID2SYM(rb_intern(name));
  KeyboardKey* key = keyboardKeys;
  while (key) {
    if (key->rbSymbol == rbNameSymbol) {
      return key;
    }
    key = key->next;
  }
  return NULL;
}

void
strb_TestInput(void)
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

  for (KeyboardKey* key = keyboardKeys; key; key = key->next) {
    assert(0 == key->state);
  }

  assert(false == IsPressed(0, -1, -1, 0));
  assert(true  == IsPressed(1, -1, -1, 0));
  assert(true  == IsPressed(2, -1, -1, 0));
  assert(true  == IsPressed(3, -1, -1, 0));
  assert(true  == IsPressed(4, -1, -1, 0));
  assert(true  == IsPressed(5, -1, -1, 0));
  assert(true  == IsPressed(100, -1, -1, 0));

  assert(false == IsPressed(0, 0, -1, 0));
  assert(false == IsPressed(1, 0, -1, 0));
  assert(false == IsPressed(2, 0, -1, 0));
  assert(false == IsPressed(3, 0, -1, 0));
  assert(false == IsPressed(4, 0, -1, 0));
  assert(false == IsPressed(5, 0, -1, 0));
  assert(false == IsPressed(100, 0, -1, 0));

  assert(false == IsPressed(0, 1, -1, 0));
  assert(true  == IsPressed(1, 1, -1, 0));
  assert(false == IsPressed(2, 1, -1, 0));
  assert(false == IsPressed(3, 1, -1, 0));
  assert(false == IsPressed(4, 1, -1, 0));
  assert(false == IsPressed(5, 1, -1, 0));
  assert(false == IsPressed(100, 1, -1, 0));

  assert(false == IsPressed(0, 3, -1, 0));
  assert(true  == IsPressed(1, 3, -1, 0));
  assert(true  == IsPressed(2, 3, -1, 0));
  assert(true  == IsPressed(3, 3, -1, 0));
  assert(false == IsPressed(4, 3, -1, 0));
  assert(false == IsPressed(5, 3, -1, 0));
  assert(false == IsPressed(100, 3, -1, 0));

  // with delay
  assert(false == IsPressed(0, -1, 3, 0));
  assert(true  == IsPressed(1, -1, 3, 0));
  assert(true  == IsPressed(2, -1, 3, 0));
  assert(true  == IsPressed(3, -1, 3, 0));
  assert(true  == IsPressed(4, -1, 3, 0));
  assert(true  == IsPressed(5, -1, 3, 0));
  assert(true  == IsPressed(100, -1, 3, 0));

  assert(false == IsPressed(0, 0, 3, 0));
  assert(false == IsPressed(1, 0, 3, 0));
  assert(false == IsPressed(2, 0, 3, 0));
  assert(false == IsPressed(3, 0, 3, 0));
  assert(false == IsPressed(4, 0, 3, 0));
  assert(false == IsPressed(5, 0, 3, 0));
  assert(false == IsPressed(100, 0, 3, 0));

  assert(false == IsPressed(0, 1, 3, 0));
  assert(true  == IsPressed(1, 1, 3, 0));
  assert(false == IsPressed(2, 1, 3, 0));
  assert(false == IsPressed(3, 1, 3, 0));
  assert(false == IsPressed(4, 1, 3, 0));
  assert(true  == IsPressed(5, 1, 3, 0));
  assert(true  == IsPressed(6, 1, 3, 0));
  assert(true  == IsPressed(7, 1, 3, 0));
  assert(true  == IsPressed(8, 1, 3, 0));
  assert(true  == IsPressed(100, 1, 3, 0));

  assert(false == IsPressed(0, 3, 3, 0));
  assert(true  == IsPressed(1, 3, 3, 0));
  assert(true  == IsPressed(2, 3, 3, 0));
  assert(true  == IsPressed(3, 3, 3, 0));
  assert(false == IsPressed(4, 3, 3, 0));
  assert(false == IsPressed(5, 3, 3, 0));
  assert(false == IsPressed(6, 3, 3, 0));
  assert(true  == IsPressed(7, 3, 3, 0));
  assert(true  == IsPressed(8, 3, 3, 0));
  assert(true  == IsPressed(100, 3, 3, 0));

  // with delay and interval
  assert(false == IsPressed(0, -1, 3, 1));
  assert(true  == IsPressed(1, -1, 3, 1));
  assert(true  == IsPressed(2, -1, 3, 1));
  assert(true  == IsPressed(3, -1, 3, 1));
  assert(true  == IsPressed(4, -1, 3, 1));
  assert(true  == IsPressed(5, -1, 3, 1));
  assert(true  == IsPressed(100, -1, 3, 1));

  assert(false == IsPressed(0, 0, 3, 1));
  assert(false == IsPressed(1, 0, 3, 1));
  assert(false == IsPressed(2, 0, 3, 1));
  assert(false == IsPressed(3, 0, 3, 1));
  assert(false == IsPressed(4, 0, 3, 1));
  assert(false == IsPressed(5, 0, 3, 1));
  assert(false == IsPressed(100, 0, 3, 1));

  assert(false == IsPressed(0, 1, 3, 1));
  assert(true  == IsPressed(1, 1, 3, 1));
  assert(false == IsPressed(2, 1, 3, 1));
  assert(false == IsPressed(3, 1, 3, 1));
  assert(false == IsPressed(4, 1, 3, 1));
  assert(true  == IsPressed(5, 1, 3, 1));
  assert(false == IsPressed(6, 1, 3, 1));
  assert(true  == IsPressed(7, 1, 3, 1));
  assert(false == IsPressed(8, 1, 3, 1));
  assert(true  == IsPressed(9, 1, 3, 1));
  assert(false == IsPressed(10, 1, 3, 1));
  assert(true  == IsPressed(11, 1, 3, 1));
  assert(false == IsPressed(12, 1, 3, 1));
  assert(false == IsPressed(100, 1, 3, 1));

  assert(false == IsPressed(0, 3, 3, 1));
  assert(true  == IsPressed(1, 3, 3, 1));
  assert(true  == IsPressed(2, 3, 3, 1));
  assert(true  == IsPressed(3, 3, 3, 1));
  assert(false == IsPressed(4, 3, 3, 1));
  assert(false == IsPressed(5, 3, 3, 1));
  assert(false == IsPressed(6, 3, 3, 1));
  assert(true  == IsPressed(7, 3, 3, 1));
  assert(false == IsPressed(8, 3, 3, 1));
  assert(true  == IsPressed(9, 3, 3, 1));
  assert(false == IsPressed(10, 3, 3, 1));
  assert(true  == IsPressed(11, 3, 3, 1));
  assert(false == IsPressed(12, 3, 3, 1));
  assert(false == IsPressed(100, 3, 3, 1));

  // with delay and interval 2
  assert(false == IsPressed(0, -1, 3, 2));
  assert(true  == IsPressed(1, -1, 3, 2));
  assert(true  == IsPressed(2, -1, 3, 2));
  assert(true  == IsPressed(3, -1, 3, 2));
  assert(true  == IsPressed(4, -1, 3, 2));
  assert(true  == IsPressed(5, -1, 3, 2));
  assert(true  == IsPressed(100, -1, 3, 2));

  assert(false == IsPressed(0, 0, 3, 2));
  assert(false == IsPressed(1, 0, 3, 2));
  assert(false == IsPressed(2, 0, 3, 2));
  assert(false == IsPressed(3, 0, 3, 2));
  assert(false == IsPressed(4, 0, 3, 2));
  assert(false == IsPressed(5, 0, 3, 2));
  assert(false == IsPressed(100, 0, 3, 2));

  assert(false == IsPressed(0, 1, 3, 2));
  assert(true  == IsPressed(1, 1, 3, 2));
  assert(false == IsPressed(2, 1, 3, 2));
  assert(false == IsPressed(3, 1, 3, 2));
  assert(false == IsPressed(4, 1, 3, 2));
  assert(true  == IsPressed(5, 1, 3, 2));
  assert(false == IsPressed(6, 1, 3, 2));
  assert(false == IsPressed(7, 1, 3, 2));
  assert(true  == IsPressed(8, 1, 3, 2));
  assert(false == IsPressed(9, 1, 3, 2));
  assert(false == IsPressed(10, 1, 3, 2));
  assert(true  == IsPressed(11, 1, 3, 2));
  assert(false == IsPressed(12, 1, 3, 2));
  assert(false == IsPressed(100, 1, 3, 2));

  assert(false == IsPressed(0, 3, 3, 2));
  assert(true  == IsPressed(1, 3, 3, 2));
  assert(true  == IsPressed(2, 3, 3, 2));
  assert(true  == IsPressed(3, 3, 3, 2));
  assert(false == IsPressed(4, 3, 3, 2));
  assert(false == IsPressed(5, 3, 3, 2));
  assert(false == IsPressed(6, 3, 3, 2));
  assert(true  == IsPressed(7, 3, 3, 2));
  assert(false == IsPressed(8, 3, 3, 2));
  assert(false == IsPressed(9, 3, 3, 2));
  assert(true  == IsPressed(10, 3, 3, 2));
  assert(false == IsPressed(11, 3, 3, 2));
  assert(false == IsPressed(12, 3, 3, 2));
  assert(true  == IsPressed(100, 3, 3, 2));

  printf("End Test: Input\n");
}
#endif
