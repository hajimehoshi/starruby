#include "starruby.h"

static SDL_Surface* screen;

static int fps = 30;
static double realFps = 0;
static bool running = false;
static bool terminated = false;

static VALUE rbInput;
static VALUE rbScreen;

static VALUE Game_fps(VALUE self)
{
  return INT2NUM(fps);
}

static VALUE Game_fps_eq(VALUE self, VALUE rbFps)
{
  fps = NUM2INT(rbFps);
  return rbFps;
}

static VALUE Game_input(VALUE self)
{
  if (rbInput == Qnil) {
    rbInput = rb_funcall(rb_cInput, rb_intern("new"), 0);
    rb_define_readonly_variable("input", &rbInput); // for GC
  }
  return rbInput;
}

static VALUE Game_real_fps(VALUE self)
{
  return rb_float_new(realFps);
}

static VALUE Game_screen(VALUE);

static VALUE Game_run(int argc, VALUE* argv, VALUE self)
{
  if (running) {
    rb_raise(rb_eStarRubyError, "already run");
    return Qnil;
  }
  running = true;
  terminated = false;
  
  VALUE block;
  rb_scan_args(argc, argv, "0&", &block);
  
  SDL_Event event;
  Uint32 now;
  int nowX;
  Uint32 before = SDL_GetTicks();
  Uint32 before2 = before;
  int errorX = 0;
  int counter = 0;

  Texture* texture;
  Data_Get_Struct(Game_screen(self), Texture, texture);
  
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
    rb_yield(Qnil);
    
    SDL_LockSurface(screen);
    MEMCPY(screen->pixels, texture->pixels, Pixel, SCREEN_WIDTH * SCREEN_HEIGHT);
    SDL_UnlockSurface(screen);
    if (SDL_Flip(screen))
      rb_raise_sdl_error();
    
    if (terminated)
      break;
  }
  running = false;
  return Qnil;
}

static VALUE Game_running(VALUE self)
{
  return running ? Qtrue : Qfalse;
}

static VALUE Game_screen(VALUE self)
{
  if (rbScreen == Qnil) {
    rbScreen = rb_funcall(rb_cTexture, rb_intern("new"), 2,
                          INT2NUM(SCREEN_WIDTH), INT2NUM(SCREEN_HEIGHT));
    rb_define_readonly_variable("screen", &rbScreen); // for GC
  }
  return rbScreen;
}

static VALUE Game_terminate(VALUE self)
{
  terminated = true;
  return Qnil;
}

static VALUE Game_title(VALUE self)
{
  return rb_iv_get(self, "title");
}

static VALUE Game_title_eq(VALUE self, VALUE rbTitle)
{
  return rb_iv_set(self, "title", rbTitle);
}

void InitializeGame(SDL_Surface* _screen)
{
  screen = _screen;
  
  rb_mGame = rb_define_module_under(rb_mStarRuby, "Game");
  rb_define_singleton_method(rb_mGame, "fps",       Game_fps,       0);
  rb_define_singleton_method(rb_mGame, "fps=",      Game_fps_eq,    1);
  rb_define_singleton_method(rb_mGame, "input",     Game_input,     0);
  rb_define_singleton_method(rb_mGame, "real_fps",  Game_real_fps,  0);
  rb_define_singleton_method(rb_mGame, "run",       Game_run,       -1);
  rb_define_singleton_method(rb_mGame, "running?",  Game_running,   0);
  rb_define_singleton_method(rb_mGame, "screen",    Game_screen,    0);
  rb_define_singleton_method(rb_mGame, "terminate", Game_terminate, 0);
  rb_define_singleton_method(rb_mGame, "title",     Game_title,     0);
  rb_define_singleton_method(rb_mGame, "title=",    Game_title_eq,  1);

  rbInput = Qnil;
  rbScreen = Qnil;
}
