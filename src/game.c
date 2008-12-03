#include "starruby_private.h"

static volatile VALUE rb_cGame     = Qundef;
static volatile VALUE rb_mStarRuby = Qundef;

static volatile VALUE symbol_cursor       = Qundef;
static volatile VALUE symbol_fps          = Qundef;
static volatile VALUE symbol_fullscreen   = Qundef;
static volatile VALUE symbol_title        = Qundef;
static volatile VALUE symbol_window_scale = Qundef;

typedef struct {
  Uint32 error;
  Uint32 before;
  Uint32 before2;
  int counter;
} GameTimer;

typedef struct {
  int windowScale;
  int realScreenWidth;
  int realScreenHeight;
  VALUE screen;
  SDL_Surface* sdlScreen;
  int fps;
  double realFps;
  GameTimer timer;
  bool isWindowClosing;
  bool isTerminated; // backward compatibility
} Game;

inline static void
CheckDisposed(const Game* const game)
{
  if (!game) {
    rb_raise(rb_eRuntimeError, "can't modify disposed StarRuby::Game");
  }
}

static VALUE Game_s_current(VALUE);
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
Game_s_fps(VALUE self)
{
  rb_warn("Game.fps is deprecated;"
          " use Game#fps instead");
  volatile VALUE rbCurrent = Game_s_current(self);
  if (!NIL_P(rbCurrent)) {
    return Game_fps(rbCurrent);
  } else {
    return rb_iv_get(self, "default_fps");
  }
}

static VALUE
Game_s_fps_eq(VALUE self, VALUE rbFps)
{
  rb_warn("Game.fps= is deprecated;"
          " use Game#fps= or Game.run(..., :fps => ...) instead");
  volatile VALUE rbCurrent = Game_s_current(self);
  if (!NIL_P(rbCurrent)) {
    return Game_fps_eq(rbCurrent, rbFps);
  } else {
    return rb_iv_set(self, "default_fps", INT2NUM(NUM2INT(rbFps)));
  }
}

static VALUE
Game_s_real_fps(VALUE self)
{
  rb_warn("Game.real_fps is deprecated;"
          " use Game#real_fps instead");
  volatile VALUE rbCurrent = Game_s_current(self);
  if (!NIL_P(rbCurrent)) {
    return Game_real_fps(rbCurrent);
  } else {
    return rb_float_new(0.0);
  }
}

static VALUE
RunGame(VALUE rbGame)
{
  const Game* game;
  Data_Get_Struct(rbGame, Game, game);
  while (true) {
    Game_update_state(rbGame);
    if (RTEST(Game_window_closing(rbGame)) || game->isTerminated) {
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
Game_s_running(VALUE self)
{
  rb_warn("Game.running? is deprecated;"
          " use Game.current instead");
  return !NIL_P(Game_s_current(self)) ? Qtrue : Qfalse;
}

static VALUE
Game_s_screen(VALUE self)
{
  rb_warn("Game.screen is deprecated;"
          " use Game#screen instead");
  volatile VALUE rbCurrent = Game_s_current(self);
  if (NIL_P(rbCurrent)) {
    return Qnil;
  }
  return Game_screen(rbCurrent);
}

static VALUE
Game_s_terminate(VALUE self)
{
  rb_warn("Game.terminate is deprecated;"
          " use break instead");
  volatile VALUE rbCurrent = Game_s_current(self);
  if (NIL_P(rbCurrent)) {
    rb_raise(strb_GetStarRubyErrorClass(), "a game has not run yet");
  }
  Game* game;
  Data_Get_Struct(rbCurrent, Game, game);
  game->isTerminated = true;
  return Qnil;
}

static VALUE
Game_s_ticks(VALUE self)
{
  return INT2NUM(SDL_GetTicks());
}

static VALUE
Game_s_title(VALUE self)
{
  rb_warn("Game.title is deprecated;"
          " use Game#title instead");
  volatile VALUE rbCurrent = Game_s_current(self);
  if (!NIL_P(rbCurrent)) {
    return Game_title(rbCurrent);
  } else {
    return rb_iv_get(self, "default_title");
  }
}

static VALUE
Game_s_title_eq(VALUE self, VALUE rbTitle)
{
  rb_warn("Game.title= is deprecated;"
          " use Game#title= or Game.run(..., :title => ...) instead");
  volatile VALUE rbCurrent = Game_s_current(self);
  if (!NIL_P(rbCurrent)) {
    return Game_title_eq(rbCurrent, rbTitle);
  } else {
    Check_Type(rbTitle, T_STRING);
    if (SDL_WasInit(SDL_INIT_VIDEO)) {
      SDL_WM_SetCaption(StringValueCStr(rbTitle), NULL);
    }
    return rb_iv_set(self, "default_title", rbTitle);
  }
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
  }
  free(game);
}

static VALUE
Game_alloc(VALUE klass)
{
  if (!NIL_P(Game_s_current(klass))) {
    rb_raise(strb_GetStarRubyErrorClass(), "already run");
  }
  Game* game = ALLOC(Game);
  game->windowScale = 1;
  game->realScreenWidth = 0;
  game->realScreenHeight = 0;
  game->screen = Qnil;
  game->sdlScreen = NULL;
  game->realFps = 0;
  game->timer.error = 0;
  game->timer.before = SDL_GetTicks();
  game->timer.before2 = game->timer.before2;
  game->timer.counter = 0;
  game->isWindowClosing = false;
  game->isTerminated = false;
  return Data_Wrap_Struct(klass, Game_mark, Game_free, game);;
}

static VALUE
Game_initialize(int argc, VALUE* argv, VALUE self)
{
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
  const int width   = NUM2INT(rbWidth);
  const int height  = NUM2INT(rbHeight);

  volatile VALUE rbFps = rb_hash_aref(rbOptions, symbol_fps);
  if (!NIL_P(rbFps)) {
    Game_fps_eq(self, rbFps);
  } else {
    // backward compatibility
    volatile VALUE rbFps2 = rb_iv_get(rb_cGame, "default_fps");
    if (!NIL_P(rbFps2)) {
      Game_fps_eq(self, rbFps2);
    } else {
      Game_fps_eq(self, INT2FIX(30));
    }
  }

  volatile VALUE rbTitle = rb_hash_aref(rbOptions, symbol_title);
  if (!NIL_P(rbTitle)) {
    Game_title_eq(self, rbTitle);
  } else {
    // backward compatibility
    volatile VALUE rbTitle2 = rb_iv_get(rb_cGame, "default_title");
    if (!NIL_P(rbTitle2)) {
      Game_title_eq(self, rbTitle2);
    } else {
      Game_title_eq(self, rb_str_new2(""));
    }
  }

  bool cursor = false;
  bool fullscreen = false;
  const int bpp = 32;
  game->windowScale = 1;

  volatile VALUE val;
  Check_Type(rbOptions, T_HASH);
  if (!NIL_P(val = rb_hash_aref(rbOptions, symbol_cursor))) {
    cursor = RTEST(val);
  }
  if (!NIL_P(val = rb_hash_aref(rbOptions, symbol_fullscreen))) {
    fullscreen = RTEST(val);
  }
  if (!NIL_P(val = rb_hash_aref(rbOptions, symbol_window_scale))) {
    game->windowScale = NUM2INT(val);
    if (game->windowScale < 1) {
      rb_raise(rb_eArgError, "invalid window scale: %d",
               game->windowScale);
    }
  }
  game->realScreenWidth  = width  * game->windowScale;
  game->realScreenHeight = height * game->windowScale;
  SDL_ShowCursor(cursor ? SDL_ENABLE : SDL_DISABLE);

  Uint32 options = 0;
  options |= SDL_DOUBLEBUF;
  if (fullscreen) {
    options |= SDL_HWSURFACE | SDL_FULLSCREEN;
    game->windowScale = 1;    
    SDL_Rect** modes = SDL_ListModes(NULL, options);
    if (!modes) {
      rb_raise(rb_eRuntimeError, "not supported fullscreen resolution");
    }
    if (modes != (SDL_Rect**)-1) {
      game->realScreenWidth = 0;
      game->realScreenHeight = 0;
      for (int i = 0; modes[i]; i++) {
        int realBpp = SDL_VideoModeOK(modes[i]->w, modes[i]->h, bpp, options);
        if (width <= modes[i]->w && height <= modes[i]->h && realBpp == bpp) {
          game->realScreenWidth  = modes[i]->w;
          game->realScreenHeight = modes[i]->h;
        } else {
          break;
        }
      }
      if (game->realScreenWidth == 0 || game->realScreenHeight == 0) {
        rb_raise(rb_eRuntimeError, "not supported fullscreen resolution");
      }
    }
  } else {
    options |= SDL_SWSURFACE;
  }

  game->sdlScreen = SDL_SetVideoMode(game->realScreenWidth,
                                     game->realScreenHeight, bpp, options);
  if (!game->sdlScreen) {
    rb_raise_sdl_error();
  }
  volatile VALUE rbScreen =
    rb_class_new_instance(2, (VALUE[]){INT2NUM(width), INT2NUM(height)},
                          strb_GetTextureClass());
  game->screen = rbScreen;

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
  const Pixel* src = texture->pixels;
  const int realScreenWidth    = game->realScreenWidth;
  const int realScreenHeight   = game->realScreenHeight;
  SDL_Surface* sdlScreen = game->sdlScreen;
  SDL_LockSurface(sdlScreen);
  Pixel* dst = (Pixel*)sdlScreen->pixels;
  const int screenPadding =
    sdlScreen->pitch / sdlScreen->format->BytesPerPixel - sdlScreen->w;
  const int textureWidth  = texture->width;
  const int textureHeight = texture->height;
  const int windowScale = game->windowScale;
  switch (windowScale) {
  case 1:
    {
      // For fullscreen
      dst += ((realScreenWidth - textureWidth) >> 1) +
        ((realScreenHeight - textureHeight) >> 1) * (realScreenWidth + screenPadding);
      const int heightPadding = realScreenWidth - texture->width + screenPadding;
      for (int j = 0; j < textureHeight; j++, dst += heightPadding) {
        for (int i = 0; i < textureWidth; i++, src++, dst++) {
          const uint8_t alpha = src->color.alpha;
          dst->color.red   = DIV255(src->color.red   * alpha);
          dst->color.green = DIV255(src->color.green * alpha);
          dst->color.blue  = DIV255(src->color.blue  * alpha);
        }
      }
    }
    break;
  case 2:
    {
      const int textureWidth2x = textureWidth << 1;
      const int heightPadding = textureWidth2x + (screenPadding << 1);
      for (int j = 0; j < textureHeight; j++, dst += heightPadding) {
        for (int i = 0; i < textureWidth; i++, src++, dst += 2) {
          const uint8_t alpha = src->color.alpha;
          dst->color.red   = DIV255(src->color.red   * alpha);
          dst->color.green = DIV255(src->color.green * alpha);
          dst->color.blue  = DIV255(src->color.blue  * alpha);
          dst[textureWidth2x + screenPadding] = dst[textureWidth2x + screenPadding + 1] = dst[1] = *dst;
        }
      }
    }
    break;
  default:
    {
      const int textureWidthN = textureWidth * windowScale;
      const int heightPadding =
        textureWidth * windowScale * (windowScale - 1) + screenPadding * windowScale;
      for (int j = 0; j < textureHeight; j++, dst += heightPadding) {
        for (int i = 0; i < textureWidth; i++, src++, dst += windowScale) {
          const uint8_t alpha = src->color.alpha;
          dst->color.red   = DIV255(src->color.red   * alpha);
          dst->color.green = DIV255(src->color.green * alpha);
          dst->color.blue  = DIV255(src->color.blue  * alpha);
          for (int k = 1; k < windowScale; k++) {
            dst[k] = *dst;
          }
          for (int l = 1; l < windowScale; l++) {
            for (int k = 0; k < windowScale; k++) {
              dst[(textureWidthN + screenPadding) * l + k] = *dst;
            }
          }
        }
      }
    }
    break;
  }
  SDL_UnlockSurface(sdlScreen);

  if (SDL_Flip(sdlScreen)) {
    rb_raise_sdl_error();
  }
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
    if (1000 <= (now - gameTimer->before) * fps + gameTimer->error) {
      gameTimer->error = MIN((now - gameTimer->before) * fps +
                             gameTimer->error - 1000, 1000);
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

VALUE
strb_InitializeGame(VALUE _rb_mStarRuby)
{
  rb_mStarRuby = _rb_mStarRuby;

  rb_cGame = rb_define_class_under(rb_mStarRuby, "Game", rb_cObject);
  rb_define_singleton_method(rb_cGame, "current",   Game_s_current,   0);
  rb_define_singleton_method(rb_cGame, "fps",       Game_s_fps,       0);
  rb_define_singleton_method(rb_cGame, "fps=",      Game_s_fps_eq,    1);
  rb_define_singleton_method(rb_cGame, "real_fps",  Game_s_real_fps,  0);
  rb_define_singleton_method(rb_cGame, "run",       Game_s_run,       -1);
  rb_define_singleton_method(rb_cGame, "running?",  Game_s_running,   0);
  rb_define_singleton_method(rb_cGame, "screen",    Game_s_screen,    0);
  rb_define_singleton_method(rb_cGame, "terminate", Game_s_terminate, 0);
  rb_define_singleton_method(rb_cGame, "ticks",     Game_s_ticks,     0);
  rb_define_singleton_method(rb_cGame, "title",     Game_s_title,     0);
  rb_define_singleton_method(rb_cGame, "title=",    Game_s_title_eq,  1);
  rb_define_alloc_func(rb_cGame, Game_alloc);
  rb_define_private_method(rb_cGame, "initialize", Game_initialize, -1);
  rb_define_method(rb_cGame, "dispose",         Game_dispose,        0);
  rb_define_method(rb_cGame, "disposed?",       Game_disposed,       0);
  rb_define_method(rb_cGame, "fps",             Game_fps,            0);
  rb_define_method(rb_cGame, "fps=",            Game_fps_eq,         1);
  rb_define_method(rb_cGame, "real_fps",        Game_real_fps,       0);
  rb_define_method(rb_cGame, "screen",          Game_screen,         0);
  rb_define_method(rb_cGame, "title",           Game_title,          0);
  rb_define_method(rb_cGame, "title=",          Game_title_eq,       1);
  rb_define_method(rb_cGame, "update_screen",   Game_update_screen,  0);
  rb_define_method(rb_cGame, "update_state",    Game_update_state,   0);
  rb_define_method(rb_cGame, "wait",            Game_wait,           0);
  rb_define_method(rb_cGame, "window_closing?", Game_window_closing, 0);
  rb_define_method(rb_cGame, "window_scale",    Game_window_scale,   0);

  symbol_cursor       = ID2SYM(rb_intern("cursor"));
  symbol_fps          = ID2SYM(rb_intern("fps"));
  symbol_fullscreen   = ID2SYM(rb_intern("fullscreen"));
  symbol_title        = ID2SYM(rb_intern("title"));
  symbol_window_scale = ID2SYM(rb_intern("window_scale"));

  // backward compatibility
  rb_iv_set(rb_cGame, "default_fps", INT2FIX(30));

  return rb_cGame;
}
