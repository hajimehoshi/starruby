#include "starruby.h"

volatile static VALUE symbol_fullscreen;
volatile static VALUE symbol_window_scale;

static int fps = 30;
static bool fullscreen = false;
static double realFps = 0;
static bool running = false;
static bool terminated = false;
static int windowScale = 1;

int
GetWindowScale(void)
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
DoLoop(SDL_Surface* screen)
{
  running = true;
  terminated = false;
  realFps = 0;
  
  volatile VALUE rbScreen  = rb_iv_get(rb_mGame, "screen");
  volatile VALUE rbScreen2 = rb_iv_get(rb_mGame, "screen2");
  Texture* texture;
  Data_Get_Struct(rbScreen, Texture, texture);
  Texture* texture2;
  Data_Get_Struct(rbScreen2, Texture, texture2);
  
  SDL_Event event;
  Uint32 now;
  int nowX;
  Uint32 before = SDL_GetTicks();
  Uint32 before2 = before;
  int errorX = 0;
  int counter = 0;

  while (true) {
    if (SDL_PollEvent(&event) != 0 && event.type == SDL_QUIT)
      break;
    counter++;
    while (true) {
      now = SDL_GetTicks();
      nowX = (now - before) * (fps / 10) + errorX;
      if (100 <= nowX)
        break;
      SDL_Delay(1);
    }
    before = now;
    errorX = nowX % 100;
    if ((now - before2) >= 1000) {
      realFps = (double)counter / (now - before2) * 1000;
      counter = 0;
      before2 = now;
    }
    UpdateAudio();
    rb_funcall(rb_mInput, rb_intern("update"), 0);
    
    rb_yield(Qnil);

    if (texture != texture2) {
      int index = (texture2->width - texture->width) / 2
        + (texture2->height - texture->height) / 2
          * texture2->width;
      int padding = texture2->width - texture->width;
      Pixel* src = texture->pixels;
      Pixel* dst = &(texture2->pixels[index]);
      for (int j = 0; j < texture->height; j++, dst += padding)
        for (int i = 0; i < texture->width; i++, src++, dst++)
          *dst = *src;
    }
    
    int length = texture2->width * texture2->height;
    Pixel* p = texture2->pixels;
    Pixel src[length];
    for (int i = 0; i < length; i++, p++) {
      src[i].color.red   = DIV255(p->color.red   * p->color.alpha);
      src[i].color.green = DIV255(p->color.green * p->color.alpha);
      src[i].color.blue  = DIV255(p->color.blue  * p->color.alpha);
      src[i].color.alpha = 255;
    }
    
    SDL_LockSurface(screen);
    Uint32* dst = screen->pixels;
    switch (windowScale) {
    case 1:
      MEMCPY(dst, src, Pixel, length);
      break;
    case 2:
      {
        Pixel* srcP = src;
        int width   = texture2->width;
        int width2x = width * 2;
        int height  = texture2->height;
        for (int j = 0; j < height; j++) {
          for (int i = 0; i < width; i++) {
            *dst = *(dst + width2x) = srcP->value;
            dst++;
            *dst = *(dst + width2x) = srcP->value;
            dst++;
            srcP++;
          }
          dst += width2x;
        }
      }
      break;
    }
    SDL_UnlockSurface(screen);
    
    if (SDL_Flip(screen))
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
DoLoopEnsure(SDL_Surface* screen)
{
  DisposeScreen(screen);
  SDL_QuitSubSystem(SDL_INIT_VIDEO | SDL_INIT_TIMER);
  running = false;
  terminated = false;
  return Qnil;
}

static VALUE Game_screen(VALUE);
static VALUE
Game_run(int argc, VALUE* argv, VALUE self)
{
  if (running) {
    rb_raise(rb_eStarRubyError, "already run");
    return Qnil;
  }

  if (SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_TIMER))
    rb_raise_sdl_error();
  volatile VALUE rbTitle = rb_iv_get(self, "title");
  SDL_WM_SetCaption(StringValuePtr(rbTitle), NULL);
  SDL_ShowCursor(SDL_DISABLE);

  volatile VALUE rbBlock, rbWidth, rbHeight, rbOptions;
  rb_scan_args(argc, argv, "21&", &rbWidth, &rbHeight, &rbOptions, &rbBlock);
  int width   = NUM2INT(rbWidth);
  int height  = NUM2INT(rbHeight);
  int width2  = 0;
  int height2 = 0;
  if (NIL_P(rbOptions))
    rbOptions = rb_hash_new();

  volatile VALUE val;
  Check_Type(rbOptions, T_HASH);
  if (!NIL_P(val = rb_hash_aref(rbOptions, symbol_fullscreen)))
    fullscreen = RTEST(val);
  if (!NIL_P(val = rb_hash_aref(rbOptions, symbol_window_scale)))
    windowScale = NORMALIZE(NUM2INT(val), 1, 2);

  Uint32 options = SDL_SWSURFACE | SDL_DOUBLEBUF;
  volatile VALUE rbScreen = rb_funcall(rb_cTexture, rb_intern("new"), 2,
                                       INT2NUM(width), INT2NUM(height));
  rb_iv_set(self, "screen",  rbScreen);
  volatile VALUE rbScreen2 = Qnil;
  if (fullscreen) {
    options |= SDL_FULLSCREEN;
    SDL_Rect** modes = SDL_ListModes(NULL, options);
    if (!modes) {
      rb_raise(rb_eRuntimeError, "not supported fullscreen resolution");
      return Qnil;
    }
    if (modes != (SDL_Rect**)-1) {
      for (int i = 0; modes[i]; i++) {
        if (width * windowScale <= modes[i]->w &&
            height * windowScale <= modes[i]->h) {
          width2  = modes[i]->w / windowScale;
          height2 = modes[i]->h / windowScale;
        } else {
          break;
        }
      }
      if (width2 == 0 || height2 == 0) {
        rb_raise(rb_eRuntimeError, "not supported fullscreen resolution");
        return Qnil;
      }
      if (width == width2 && height == height2)
        rbScreen2 = rbScreen;
      else
        rbScreen2 = rb_funcall(rb_cTexture, rb_intern("new"), 2,
                               INT2NUM(width2), INT2NUM(height2));
    }
  }
  if (NIL_P(rbScreen2))
    rbScreen2 = rbScreen;
  rb_iv_set(self, "screen2", rbScreen2);
  int screenWidth  = NUM2INT(rb_funcall(rbScreen2, rb_intern("width"), 0))
    * windowScale;
  int screenHeight = NUM2INT(rb_funcall(rbScreen2, rb_intern("height"), 0))
    * windowScale;
  SDL_Surface* screen = SDL_SetVideoMode(screenWidth, screenHeight, 32, options);
  if (!screen) {
    DisposeScreen(screen);
    rb_raise_sdl_error();
  }
  rb_ensure(DoLoop, (VALUE)screen, DoLoopEnsure, (VALUE)screen);
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
    rb_raise(rb_eStarRubyError, "A game has not run yet");
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

void
InitializeGame(void)
{
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

  symbol_fullscreen   = ID2SYM(rb_intern("fullscreen"));
  symbol_window_scale = ID2SYM(rb_intern("window_scale"));

  rb_iv_set(rb_mGame, "title", rb_str_new2(""));
}
