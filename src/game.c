#include "starruby.h"
#include "starruby_private.h"

static volatile VALUE rb_mGame;
static volatile VALUE rb_mStarRuby;

static volatile VALUE symbol_cursor;
static volatile VALUE symbol_fullscreen;
static volatile VALUE symbol_window_scale;

static int fps = 30;
static double realFps = 0;
static bool running = false;
static int screenWidth = 0;
static int screenHeight = 0;
static bool terminated = false;
static int windowScale = 1;

static SDL_Surface* sdlScreen = NULL;

int
strb_GetWindowScale(void)
{
  return windowScale;
}

static VALUE
Game_fps(VALUE self)
{
  return INT2NUM(fps);
}

static VALUE
Game_fps_eq(VALUE self, VALUE rbFps)
{
  fps = NUM2INT(rbFps);
  return rbFps;
}

static VALUE
Game_real_fps(VALUE self)
{
  return rb_float_new(realFps);
}

static VALUE
DoLoop(void)
{
  running = true;
  terminated = false;
  realFps = 0;
  
  volatile VALUE rbScreen = rb_iv_get(rb_mGame, "screen");
  Texture* texture;
  Data_Get_Struct(rbScreen, Texture, texture);

  SDL_Event event;
  Uint32 now;
  Uint32 error = 0;
  Uint32 before = SDL_GetTicks();
  Uint32 before2 = before;
  int counter = 0;

  while (true) {
    if (SDL_PollEvent(&event) && event.type == SDL_QUIT)
      break;

    int beforeFps = fps;
    while (true) {
      now = SDL_GetTicks();
      if (1000 <= (now - before) * fps + error) {
        error = (now - before) * fps + error - 1000;
        before = now;
        break;
      }
      SDL_Delay(0);
    }

    counter++;    
    if (1000 <= now - before2) {
      realFps = (double)counter * 1000 / (now - before2);
      counter = 0;
      before2 = SDL_GetTicks();
    }

    strb_UpdateAudio();
    strb_UpdateInput();
    
    rb_yield(Qnil);
    if (fps != beforeFps)
      error = 0;

    Pixel* src = texture->pixels;
    SDL_LockSurface(sdlScreen);
#ifndef GP2X
    Pixel* dst = (Pixel*)sdlScreen->pixels;
    int screenPadding =
      sdlScreen->pitch / sdlScreen->format->BytesPerPixel - sdlScreen->w;
    switch (windowScale) {
    case 1:
      // For fullscreen mode
      dst += (screenWidth - texture->width) / 2
        + (screenHeight - texture->height) / 2 * (screenWidth + screenPadding);
      int padding = screenWidth - texture->width + screenPadding;
      for (int j = 0; j < texture->height; j++, dst += padding) {
        for (int i = 0; i < texture->width; i++, src++, dst++) {
          uint8_t alpha = src->color.alpha;
          dst->color.red   = DIV255(src->color.red   * alpha);
          dst->color.green = DIV255(src->color.green * alpha);
          dst->color.blue  = DIV255(src->color.blue  * alpha);
        }
      }
      break;
    case 2:
      {
        int width   = texture->width;
        int width2x = width * 2;
        int height  = texture->height;
        for (int j = 0; j < height; j++, dst += width2x + screenPadding * 2) {
          for (int i = 0; i < width; i++, src++, dst += 2) {
            uint8_t alpha = src->color.alpha;
            dst->color.red   = DIV255(src->color.red   * alpha);
            dst->color.green = DIV255(src->color.green * alpha);
            dst->color.blue  = DIV255(src->color.blue  * alpha);
            dst[width2x + screenPadding] = dst[width2x + screenPadding + 1] = dst[1] = *dst;
          }
        }
      }
      break;
    default:
      {
        int width  = texture->width;
        int widthN = width * windowScale;
        int heightPadding =
          width * windowScale * (windowScale - 1) + screenPadding * windowScale;
        int height  = texture->height;
        for (int j = 0; j < height; j++, dst += heightPadding) {
          for (int i = 0; i < width; i++, src++, dst += windowScale) {
            uint8_t alpha = src->color.alpha;
            dst->color.red   = DIV255(src->color.red   * alpha);
            dst->color.green = DIV255(src->color.green * alpha);
            dst->color.blue  = DIV255(src->color.blue  * alpha);
            for (int k = 1; k < windowScale; k++)
              dst[k] = *dst;
            for (int l = 1; l < windowScale; l++)
              for (int k = 0; k < windowScale; k++)
                dst[(widthN + screenPadding) * l + k] = *dst;
          }
        }
      }
      break;
    }
#else
    uint16_t* dst = (uint16_t*)sdlScreen->pixels;
    int length = texture->width * texture->height;
    for (int i = 0; i < length; i++, src++, dst++) {
      uint8_t alpha = src->color.alpha;
      *dst = (uint16_t)((DIV255(src->color.red   * alpha) >> 3) << 11 |
                        (DIV255(src->color.green * alpha) >> 2) << 5 |
                        (DIV255(src->color.blue  * alpha) >> 3));
    }
#endif
    SDL_UnlockSurface(sdlScreen);
    if (SDL_Flip(sdlScreen))
      rb_raise_sdl_error();

    if (terminated)
      break;
  }
  return Qnil;
}

static VALUE
DisposeScreen(SDL_Surface* screen)
{
  screen = NULL;
  volatile VALUE rbScreen = rb_iv_get(rb_mGame, "screen");
  if (!NIL_P(rbScreen))
    rb_funcall(rbScreen, rb_intern("dispose"), 0);
  rb_iv_set(rb_mGame, "screen", Qnil);
  return Qnil;
}

static VALUE
DoLoopEnsure()
{
  DisposeScreen(sdlScreen);
  sdlScreen = NULL;
  SDL_QuitSubSystem(SDL_INIT_VIDEO | SDL_INIT_TIMER);
  running = false;
  terminated = false;
  return Qnil;
}

static VALUE Game_screen(VALUE);
static VALUE
Game_run(int argc, VALUE* argv, VALUE self)
{
  if (running)
    rb_raise(strb_GetStarRubyErrorClass(), "already run");

  if (SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_TIMER))
    rb_raise_sdl_error();
  volatile VALUE rbTitle = rb_iv_get(self, "title");
  SDL_WM_SetCaption(StringValuePtr(rbTitle), NULL);
  volatile VALUE rbBlock, rbWidth, rbHeight, rbOptions;
  rb_scan_args(argc, argv, "21&", &rbWidth, &rbHeight, &rbOptions, &rbBlock);
#ifndef GP2X
  int width   = NUM2INT(rbWidth);
  int height  = NUM2INT(rbHeight);
#else
  int width   = 320;
  int height  = 240;
#endif
  screenWidth  = width;
  screenHeight = height;
  if (NIL_P(rbOptions))
    rbOptions = rb_hash_new();

  bool cursor = false;
  bool fullscreen = false;
  windowScale = 1;
#ifndef GP2X
  int bpp = 32;
#else
  int bpp = 16;
#endif

#ifndef GP2X
  volatile VALUE val;
  Check_Type(rbOptions, T_HASH);
  if (!NIL_P(val = rb_hash_aref(rbOptions, symbol_cursor)))
    cursor = RTEST(val);
  if (!NIL_P(val = rb_hash_aref(rbOptions, symbol_fullscreen)))
    fullscreen = RTEST(val);
  if (!NIL_P(val = rb_hash_aref(rbOptions, symbol_window_scale))) {
    windowScale = NUM2INT(val);
    if (windowScale < 1)
      rb_raise(rb_eArgError, "invalid window scale: %d", windowScale);
  }
#endif
  SDL_ShowCursor(cursor ? SDL_ENABLE : SDL_DISABLE);

  Uint32 options = 0;
#ifndef GP2X
  options |= SDL_DOUBLEBUF;
#endif
  if (fullscreen) {
    options |= SDL_HWSURFACE | SDL_FULLSCREEN;
    windowScale = 1;    
    SDL_Rect** modes = SDL_ListModes(NULL, options);
    if (!modes)
      rb_raise(rb_eRuntimeError, "not supported fullscreen resolution");
    if (modes != (SDL_Rect**)-1) {
      screenWidth = 0;
      screenHeight = 0;
      for (int i = 0; modes[i]; i++) {
        int realBpp = SDL_VideoModeOK(modes[i]->w, modes[i]->h, bpp, options);
        if (width <= modes[i]->w && height <= modes[i]->h && realBpp == bpp) {
          screenWidth  = modes[i]->w;
          screenHeight = modes[i]->h;
        } else {
          break;
        }
      }
      if (screenWidth == 0 || screenHeight == 0)
        rb_raise(rb_eRuntimeError, "not supported fullscreen resolution");
    }
  } else {
    options |= SDL_SWSURFACE;
  }

  volatile VALUE rbScreen =
    rb_class_new_instance(2, (VALUE[]){INT2NUM(width), INT2NUM(height)},
                          strb_GetTextureClass());
  rb_iv_set(self, "screen", rbScreen);

  sdlScreen = SDL_SetVideoMode(screenWidth * windowScale,
                               screenHeight * windowScale, bpp, options);
  if (!sdlScreen) {
    DisposeScreen(sdlScreen);
    rb_raise_sdl_error();
  }
  rb_ensure(DoLoop, Qnil, DoLoopEnsure, Qnil);
  return Qnil;
}

static VALUE
Game_running(VALUE self)
{
  return running ? Qtrue : Qfalse;
}

static VALUE
Game_screen(VALUE self)
{
  return rb_iv_get(self, "screen");
}

static VALUE
Game_terminate(VALUE self)
{
  if (!running)
    rb_raise(strb_GetStarRubyErrorClass(), "A game has not run yet");
  terminated = true;
  return Qnil;
}

static VALUE
Game_title(VALUE self)
{
  return rb_iv_get(self, "title");
}

static VALUE
Game_title_eq(VALUE self, VALUE rbTitle)
{
  Check_Type(rbTitle, T_STRING);
  if (SDL_WasInit(SDL_INIT_VIDEO))
    SDL_WM_SetCaption(StringValuePtr(rbTitle), NULL);
  return rb_iv_set(self, "title", rbTitle);
}

VALUE
strb_InitializeGame(VALUE _rb_mStarRuby)
{
  rb_mStarRuby = _rb_mStarRuby;

  rb_mGame = rb_define_module_under(rb_mStarRuby, "Game");
  rb_define_module_function(rb_mGame, "fps",       Game_fps,       0);
  rb_define_module_function(rb_mGame, "fps=",      Game_fps_eq,    1);
  rb_define_module_function(rb_mGame, "real_fps",  Game_real_fps,  0);
  rb_define_module_function(rb_mGame, "run",       Game_run,       -1);
  rb_define_module_function(rb_mGame, "running?",  Game_running,   0);
  rb_define_module_function(rb_mGame, "screen",    Game_screen,    0);
  rb_define_module_function(rb_mGame, "terminate", Game_terminate, 0);
  rb_define_module_function(rb_mGame, "title",     Game_title,     0);
  rb_define_module_function(rb_mGame, "title=",    Game_title_eq,  1);

  symbol_cursor       = ID2SYM(rb_intern("cursor"));
  symbol_fullscreen   = ID2SYM(rb_intern("fullscreen"));
  symbol_window_scale = ID2SYM(rb_intern("window_scale"));

  rb_iv_set(rb_mGame, "title", rb_str_new2(""));

  return rb_mGame;
}