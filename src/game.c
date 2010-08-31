#include "starruby_private.h"

static volatile VALUE rb_cGame     = Qundef;
static volatile VALUE rb_mStarRuby = Qundef;

static volatile VALUE symbol_cursor       = Qundef;
static volatile VALUE symbol_fps          = Qundef;
static volatile VALUE symbol_fullscreen   = Qundef;
static volatile VALUE symbol_title        = Qundef;
static volatile VALUE symbol_vsync        = Qundef;
static volatile VALUE symbol_window_scale = Qundef;

typedef struct {
  Uint32 error;
  Uint32 before;
  Uint32 before2;
  int counter;
} GameTimer;

typedef struct {
  int windowScale;
  bool isFullscreen;
  VALUE screen;
  SDL_Surface* sdlScreen;
  SDL_Surface* sdlScreenBuffer;
  GLuint glScreen;
  int fps;
  double realFps;
  GameTimer timer;
  bool isWindowClosing;
  bool isVsync;
} Game;

inline static void
CheckDisposed(const Game* const game)
{
  if (!game) {
    rb_raise(rb_eRuntimeError, "can't modify disposed StarRuby::Game");
  }
}

inline static int
Power2(int x)
{
  int result = 1;
  while (result < x) {
    result <<= 1;
  }
  return result;
}

static VALUE Game_s_current(VALUE);

void
strb_GetRealScreenSize(int* width, int* height)
{
  volatile VALUE rbCurrent = Game_s_current(rb_cGame);
  if (!NIL_P(rbCurrent)) {
    const Game* game;
    Data_Get_Struct(rbCurrent, Game, game);
    *width  = game->sdlScreen->w;
    *height = game->sdlScreen->h;
  } else {
    *width  = 0;
    *height = 0;
  }
}

void
strb_GetScreenSize(int* width, int* height)
{
  volatile VALUE rbCurrent = Game_s_current(rb_cGame);
  if (!NIL_P(rbCurrent)) {
    const Game* game;
    Data_Get_Struct(rbCurrent, Game, game);
    volatile VALUE rbScreen = game->screen;
    const Texture* screen;
    Data_Get_Struct(rbScreen, Texture, screen);
    if (!strb_IsDisposedTexture(screen)) {
      *width  = screen->width;
      *height = screen->height;
    } else {
      *width  = 0;
      *height = 0;
    }
  } else {
    *width  = 0;
    *height = 0;
  }
}

int
strb_GetWindowScale(void)
{
  volatile VALUE rbCurrent = Game_s_current(rb_cGame);
  if (!NIL_P(rbCurrent)) {
    const Game* game;
    Data_Get_Struct(rbCurrent, Game, game);
    return game->windowScale;
  } else {
    return 1;
  }
}

static VALUE Game_dispose(VALUE);
static VALUE Game_fps(VALUE);
static VALUE Game_fps_eq(VALUE, VALUE);
static VALUE Game_screen(VALUE);
static VALUE Game_title(VALUE);
static VALUE Game_title_eq(VALUE, VALUE);
static VALUE Game_real_fps(VALUE);
static VALUE Game_update_screen(VALUE);
static VALUE Game_update_state(VALUE);
static VALUE Game_wait(VALUE);
static VALUE Game_window_closing(VALUE);

static VALUE
Game_s_current(VALUE self)
{
  return rb_iv_get(self, "current");
}

static VALUE
RunGame(VALUE rbGame)
{
  const Game* game;
  Data_Get_Struct(rbGame, Game, game);
  while (true) {
    Game_update_state(rbGame);
    if (RTEST(Game_window_closing(rbGame))) {
      break;
    }
    rb_yield(rbGame);
    Game_update_screen(rbGame);
    Game_wait(rbGame);
  }
  return Qnil;
}

static VALUE
RunGameEnsure(VALUE rbGame)
{
  Game_dispose(rbGame);
  return Qnil;
}

static VALUE
Game_s_run(int argc, VALUE* argv, VALUE self)
{
  volatile VALUE rbGame = rb_funcall2(self, rb_intern("new"), argc, argv);
  rb_ensure(RunGame, rbGame, RunGameEnsure, rbGame);
  return Qnil;
}

static VALUE
Game_s_ticks(VALUE self)
{
  return INT2NUM(SDL_GetTicks());
}

static void
Game_mark(Game* game)
{
  if (game && !NIL_P(game->screen)) {
    rb_gc_mark(game->screen);
  }
}

static void
Game_free(Game* game)
{
  // should NOT to call SDL_FreeSurface
  if (game) {
    game->sdlScreen = NULL;
    if (game->sdlScreenBuffer) {
      SDL_FreeSurface(game->sdlScreenBuffer);
      game->sdlScreenBuffer = NULL;
    }
  }
  free(game);
}

static VALUE
Game_alloc(VALUE klass)
{
  // do not call rb_raise in this function
  Game* game = ALLOC(Game);
  game->windowScale = 1;
  game->isFullscreen = false;
  game->screen = Qnil;
  game->sdlScreen = NULL;
  game->sdlScreenBuffer = NULL;
  game->glScreen = 0;
  game->realFps = 0;
  game->timer.error = 0;
  game->timer.before = SDL_GetTicks();
  game->timer.before2 = game->timer.before2;
  game->timer.counter = 0;
  game->isWindowClosing = false;
  game->isVsync = false;
  return Data_Wrap_Struct(klass, Game_mark, Game_free, game);;
}

static void
InitializeScreen(Game* game)
{
  const int bpp = 32;

  VALUE rbScreen = game->screen;
  const Texture* screen;
  Data_Get_Struct(rbScreen, Texture, screen);
  strb_CheckDisposedTexture(screen);
  const int width  = screen->width;
  const int height = screen->height;
  int screenWidth = 0;
  int screenHeight = 0;

  Uint32 options = 0;
  options |= SDL_OPENGL;
  if (game->isFullscreen) {
    options |= SDL_HWSURFACE | SDL_FULLSCREEN;
    game->windowScale = 1;    
    SDL_Rect** modes = SDL_ListModes(NULL, options);
    if (!modes) {
      rb_raise(rb_eRuntimeError, "not supported fullscreen resolution");
    }
    if (modes != (SDL_Rect**)-1) {
      for (int i = 0; modes[i]; i++) {
        int realBpp = SDL_VideoModeOK(modes[i]->w, modes[i]->h, bpp, options);
        if (width <= modes[i]->w && height <= modes[i]->h && realBpp == bpp) {
          screenWidth  = modes[i]->w;
          screenHeight = modes[i]->h;
        } else {
          break;
        }
      }
      if (screenWidth == 0 || screenHeight == 0) {
        rb_raise(rb_eRuntimeError, "not supported fullscreen resolution");
      }
    } else {
      // any resolution are available
      screenWidth  = width;
      screenHeight = height;
    }
  } else {
    screenWidth  = width  * game->windowScale;
    screenHeight = height * game->windowScale;
    options |= SDL_SWSURFACE;
  }

  SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, game->isVsync ? 1 : 0);

  game->sdlScreen = SDL_SetVideoMode(screenWidth, screenHeight,
                                     bpp, options);
  if (!game->sdlScreen) {
    rb_raise_sdl_error();
  }
  SDL_PixelFormat* format = game->sdlScreen->format;
  game->sdlScreenBuffer = SDL_CreateRGBSurface(SDL_SWSURFACE,
                                               Power2(width),
                                               Power2(height),
                                               bpp,
                                               format->Bmask, format->Gmask, format->Bmask, format->Amask);
  if (!game->sdlScreenBuffer) {
    rb_raise_sdl_error();
  }

  glClearColor(0.0, 0.0, 0.0, 0.0);
  glOrtho(0.0, screenWidth, screenHeight, 0.0, -1.0, 1.0);
  glEnable(GL_TEXTURE_2D);
  glGenTextures(1, &game->glScreen);
  glBindTexture(GL_TEXTURE_2D, game->glScreen);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

static VALUE
Game_initialize(int argc, VALUE* argv, VALUE self)
{
  if (!NIL_P(Game_s_current(rb_cGame))) {
    rb_raise(strb_GetStarRubyErrorClass(), "already run");
  }

  volatile VALUE rbWidth, rbHeight, rbOptions;
  rb_scan_args(argc, argv, "21", &rbWidth, &rbHeight, &rbOptions);
  if (NIL_P(rbOptions)) {
    rbOptions = rb_hash_new();
  } else {
    Check_Type(rbOptions, T_HASH);
  }
  Game* game;
  Data_Get_Struct(self, Game, game);

  if (SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_TIMER)) {
    rb_raise_sdl_error();
  }

  const int width  = NUM2INT(rbWidth);
  const int height = NUM2INT(rbHeight);

  volatile VALUE rbFps = rb_hash_aref(rbOptions, symbol_fps);
  Game_fps_eq(self, !NIL_P(rbFps) ? rbFps : INT2FIX(30));

  volatile VALUE rbTitle = rb_hash_aref(rbOptions, symbol_title);
  Game_title_eq(self, !NIL_P(rbTitle) ? rbTitle : rb_str_new2(""));

  bool cursor = false;

  volatile VALUE val;
  Check_Type(rbOptions, T_HASH);
  if (!NIL_P(val = rb_hash_aref(rbOptions, symbol_cursor))) {
    cursor = RTEST(val);
  }
  if (!NIL_P(val = rb_hash_aref(rbOptions, symbol_fullscreen))) {
    game->isFullscreen = RTEST(val);
  }
  if (!NIL_P(val = rb_hash_aref(rbOptions, symbol_window_scale))) {
    game->windowScale = NUM2INT(val);
    if (game->windowScale < 1) {
      rb_raise(rb_eArgError, "invalid window scale: %d",
               game->windowScale);
    }
  }
  if (!NIL_P(val = rb_hash_aref(rbOptions, symbol_vsync))) {
    game->isVsync = RTEST(val);
  }

  SDL_ShowCursor(cursor ? SDL_ENABLE : SDL_DISABLE);

  volatile VALUE rbScreen =
    rb_class_new_instance(2, (VALUE[]){INT2NUM(width), INT2NUM(height)},
                          strb_GetTextureClass());
  game->screen = rbScreen;

  InitializeScreen(game);

  rb_iv_set(rb_cGame, "current", self);

  return Qnil;
}

static VALUE
Game_dispose(VALUE self)
{
  Game* game;
  Data_Get_Struct(self, Game, game);
  DATA_PTR(self) = NULL;
  if (game) {
    volatile VALUE rbScreen = game->screen;
    if (!NIL_P(rbScreen)) {
      rb_funcall(rbScreen, rb_intern("dispose"), 0);
      game->screen = Qnil;
    }
    if (game->glScreen) {
      glDeleteTextures(1, &game->glScreen);
      game->glScreen = 0;
    }
  }
  SDL_QuitSubSystem(SDL_INIT_VIDEO | SDL_INIT_TIMER);
  rb_iv_set(rb_cGame, "current", Qnil);
  return Qnil;
}

static VALUE
Game_disposed(VALUE self)
{
  const Game* game;
  Data_Get_Struct(self, Game, game);
  return !game ? Qtrue : Qfalse;
}

static VALUE
Game_fps(VALUE self)
{
  const Game* game;
  Data_Get_Struct(self, Game, game);
  CheckDisposed(game);
  return INT2NUM(game->fps);
}

static VALUE
Game_fps_eq(VALUE self, VALUE rbFps)
{
  Game* game;
  Data_Get_Struct(self, Game, game);
  CheckDisposed(game);
  game->fps = NUM2INT(rbFps);
  return rbFps;
}

static VALUE
Game_fullscreen(VALUE self)
{
  Game* game;
  Data_Get_Struct(self, Game, game);
  CheckDisposed(game);
  return game->isFullscreen ? Qtrue : Qfalse;
}

static VALUE
Game_fullscreen_eq(VALUE self, VALUE rbFullscreen)
{
  Game* game;
  Data_Get_Struct(self, Game, game);
  CheckDisposed(game);
  game->isFullscreen = RTEST(rbFullscreen);
  InitializeScreen(game);
  return Qnil;
}

static VALUE
Game_real_fps(VALUE self)
{
  const Game* game;
  Data_Get_Struct(self, Game, game);
  CheckDisposed(game);
  return rb_float_new(game->realFps);
}

static VALUE
Game_screen(VALUE self)
{
  const Game* game;
  Data_Get_Struct(self, Game, game);
  CheckDisposed(game);
  return game->screen;
}

static VALUE
Game_title(VALUE self)
{
  const Game* game;
  Data_Get_Struct(self, Game, game);
  CheckDisposed(game);
  return rb_iv_get(self, "title");
}

static VALUE
Game_title_eq(VALUE self, VALUE rbTitle)
{
  Game* game;
  Data_Get_Struct(self, Game, game);
  CheckDisposed(game);
  Check_Type(rbTitle, T_STRING);
  if (SDL_WasInit(SDL_INIT_VIDEO)) {
    SDL_WM_SetCaption(StringValueCStr(rbTitle), NULL);
  }
  return rb_iv_set(self, "title", rb_str_dup(rbTitle));
}

static VALUE
Game_update_screen(VALUE self)
{
  const Game* game;
  Data_Get_Struct(self, Game, game);
  CheckDisposed(game);

  volatile VALUE rbScreen = game->screen;
  const Texture* texture;
  Data_Get_Struct(rbScreen, Texture, texture);
  strb_CheckDisposedTexture(texture);
  const Pixel* src = texture->pixels;
  SDL_Surface* sdlScreenBuffer = game->sdlScreenBuffer;
  SDL_LockSurface(sdlScreenBuffer);
  Pixel* dst = (Pixel*)sdlScreenBuffer->pixels;
  const int screenPadding =
    sdlScreenBuffer->pitch / sdlScreenBuffer->format->BytesPerPixel - sdlScreenBuffer->w;
  const int textureWidth  = texture->width;
  const int textureHeight = texture->height;
  const int heightPadding = sdlScreenBuffer->w - texture->width + screenPadding;
  for (int j = 0; j < textureHeight; j++, dst += heightPadding) {
    for (int i = 0; i < textureWidth; i++, src++, dst++) {
      const uint8_t alpha = src->color.alpha;
      if (alpha == 255) {
        *dst = *src;
      } else if (alpha) {
        dst->color.red   = DIV255(src->color.red   * alpha);
        dst->color.green = DIV255(src->color.green * alpha);
        dst->color.blue  = DIV255(src->color.blue  * alpha);
      } else {
        dst->color.red   = 0;
        dst->color.green = 0;
        dst->color.blue  = 0;
      }
    }
  }

  SDL_UnlockSurface(sdlScreenBuffer);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
               sdlScreenBuffer->w, sdlScreenBuffer->h,
               0, GL_BGRA, GL_UNSIGNED_BYTE, sdlScreenBuffer->pixels);
  glClear(GL_COLOR_BUFFER_BIT);
  glColor3f(1.0, 1.0, 1.0);
  glBegin(GL_QUADS);
  {
    int x1, y1, x2, y2;
    if (!game->isFullscreen) {
      x1 = 0;
      y1 = 0;
      x2 = game->sdlScreen->w;
      y2 = game->sdlScreen->h;
    } else {
      x1 = (game->sdlScreen->w - textureWidth)  / 2;
      y1 = (game->sdlScreen->h - textureHeight) / 2;
      x2 = x1 + textureWidth;
      y2 = y1 + textureHeight;
    }
    const double tu = (double)textureWidth  / sdlScreenBuffer->w;
    const double tv = (double)textureHeight / sdlScreenBuffer->h;
    glTexCoord2f(0.0, 0.0);
    glVertex3i(x1, y1, 0);
    glTexCoord2f(tu, 0.0);
    glVertex3i(x2, y1, 0);
    glTexCoord2f(tu, tv);
    glVertex3i(x2, y2, 0);
    glTexCoord2f(0.0, tv);
    glVertex3i(x1, y2, 0);
  }
  glEnd();

  SDL_GL_SwapBuffers();
  return Qnil;
}

static VALUE
Game_update_state(VALUE self)
{
  Game* game;
  Data_Get_Struct(self, Game, game);
  CheckDisposed(game);
  SDL_Event event;
  game->isWindowClosing = (SDL_PollEvent(&event) && event.type == SDL_QUIT);
  strb_UpdateInput();
  return Qnil;
}

static VALUE
Game_wait(VALUE self)
{
  Game* game;
  Data_Get_Struct(self, Game, game);
  CheckDisposed(game);
  GameTimer* gameTimer = &(game->timer);
  const unsigned int fps = game->fps;
  Uint32 now;
  while (true) {
    now = SDL_GetTicks();
    Uint32 diff = (now - gameTimer->before) * fps + gameTimer->error;
    if (1000 <= diff) {
      gameTimer->error = MIN(diff - 1000, 1000);
      gameTimer->before = now;
      break;
    }        
    SDL_Delay(1);
  }
  gameTimer->counter++;
  if (1000 <= now - gameTimer->before2) {
    game->realFps = gameTimer->counter * 1000.0 /
      (now - gameTimer->before2);
    gameTimer->counter = 0;
    gameTimer->before2 = SDL_GetTicks();
  }
  return Qnil;
}

static VALUE
Game_window_closing(VALUE self)
{
  const Game* game;
  Data_Get_Struct(self, Game, game);
  CheckDisposed(game);
  return game->isWindowClosing ? Qtrue : Qfalse;
}

static VALUE
Game_window_scale(VALUE self)
{
  const Game* game;
  Data_Get_Struct(self, Game, game);
  CheckDisposed(game);
  return INT2FIX(game->windowScale);
}

static VALUE
Game_window_scale_eq(VALUE self, VALUE rbWindowScale)
{
  Game* game;
  Data_Get_Struct(self, Game, game);
  CheckDisposed(game);
  game->windowScale = NUM2INT(rbWindowScale);
  InitializeScreen(game);
  return Qnil;
}

VALUE
strb_InitializeGame(VALUE _rb_mStarRuby)
{
  rb_mStarRuby = _rb_mStarRuby;

  rb_cGame = rb_define_class_under(rb_mStarRuby, "Game", rb_cObject);
  rb_define_singleton_method(rb_cGame, "current",   Game_s_current,   0);
  rb_define_singleton_method(rb_cGame, "run",       Game_s_run,       -1);
  rb_define_singleton_method(rb_cGame, "ticks",     Game_s_ticks,     0);
  rb_define_alloc_func(rb_cGame, Game_alloc);
  rb_define_private_method(rb_cGame, "initialize", Game_initialize, -1);
  rb_define_method(rb_cGame, "dispose",         Game_dispose,         0);
  rb_define_method(rb_cGame, "disposed?",       Game_disposed,        0);
  rb_define_method(rb_cGame, "fps",             Game_fps,             0);
  rb_define_method(rb_cGame, "fps=",            Game_fps_eq,          1);
  rb_define_method(rb_cGame, "fullscreen?",     Game_fullscreen,      0);
  rb_define_method(rb_cGame, "fullscreen=",     Game_fullscreen_eq,   1);
  rb_define_method(rb_cGame, "real_fps",        Game_real_fps,        0);
  rb_define_method(rb_cGame, "screen",          Game_screen,          0);
  rb_define_method(rb_cGame, "title",           Game_title,           0);
  rb_define_method(rb_cGame, "title=",          Game_title_eq,        1);
  rb_define_method(rb_cGame, "update_screen",   Game_update_screen,   0);
  rb_define_method(rb_cGame, "update_state",    Game_update_state,    0);
  rb_define_method(rb_cGame, "wait",            Game_wait,            0);
  rb_define_method(rb_cGame, "window_closing?", Game_window_closing,  0);
  rb_define_method(rb_cGame, "window_scale",    Game_window_scale,    0);
  rb_define_method(rb_cGame, "window_scale=",   Game_window_scale_eq, 1);

  symbol_cursor       = ID2SYM(rb_intern("cursor"));
  symbol_fps          = ID2SYM(rb_intern("fps"));
  symbol_fullscreen   = ID2SYM(rb_intern("fullscreen"));
  symbol_title        = ID2SYM(rb_intern("title"));
  symbol_vsync        = ID2SYM(rb_intern("vsync"));
  symbol_window_scale = ID2SYM(rb_intern("window_scale"));

  return rb_cGame;
}
