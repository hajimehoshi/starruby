#include "starruby.h"

static int _fps = 30;
static double _realFps = 0;
static bool _running = false;
static bool _terminated = false;

static VALUE game_fps(VALUE self)
{
  return INT2NUM(_fps);
}

static VALUE game_fps_eq(VALUE self, VALUE fps)
{
  _fps = NUM2INT(fps);
  return fps;
}

static VALUE game_real_fps(VALUE self)
{
  return rb_float_new(_realFps);
}

static VALUE game_run(int argc, VALUE* argv, VALUE self)
{
  if (_running) {
    rb_raise(rb_eStarRubyError, "already run");
    return Qnil;
  }
  
  _running = true;
  _terminated = false;
  
  VALUE block;
  rb_scan_args(argc, argv, "0&", &block);

  Uint32 flags = SDL_INIT_VIDEO | SDL_INIT_JOYSTICK |
    SDL_INIT_AUDIO | SDL_INIT_TIMER;
  if (SDL_Init(flags) < 0)
    rb_raise_sdl_error();
  
  SDL_ShowCursor(SDL_DISABLE);

  Uint32 options = SDL_HWACCEL | SDL_DOUBLEBUF;

  SDL_Surface* screen = SDL_SetVideoMode(320, 240, 32, options);
  if (screen == NULL)
    rb_raise_sdl_error();

  SDL_Event event;
  Uint32 now;
  int nowX;
  Uint32 before = SDL_GetTicks();
  Uint32 before2 = before;
  int errorX = 0;
  int perSecondCounter = 0;
  
  while (true) {
    if (SDL_PollEvent(&event) != 0 && event.type == SDL_QUIT)
      break;

    perSecondCounter++;
    
    while (true) {
      now = SDL_GetTicks();
      nowX = (now - before) * (_fps / 10) + errorX;
      if (100 <= nowX)
        break;
      SDL_Delay(1);
    }
    before = now;
    errorX = nowX % 100;

    if ((now - before2) >= 1000) {
      _realFps = (double)perSecondCounter / (now - before2) * 1000;
      perSecondCounter = 0;
      before2 = now;
    }
    
    rb_yield(Qnil);

    if (_terminated)
      break;
  }

  SDL_FreeSurface(screen);
  
  SDL_Quit();
  
  _running = false;
  
  return Qnil;
}

static VALUE game_running(VALUE self)
{
  return _running ? Qtrue : Qfalse;
}

static VALUE game_terminate(VALUE self)
{
  _terminated = true;
  return Qnil;
}

static VALUE game_title(VALUE self)
{
  return rb_iv_get(self, "title");
}

static VALUE game_title_eq(VALUE self, VALUE title)
{
  return rb_iv_set(self, "title", title);
}

void init_game(void)
{
  VALUE rb_mGame = rb_define_module_under(rb_mStarRuby, "Game");
  rb_define_singleton_method(rb_mGame, "fps",       game_fps,       0);
  rb_define_singleton_method(rb_mGame, "fps=",      game_fps_eq,    1);
  rb_define_singleton_method(rb_mGame, "real_fps",  game_real_fps,  0);
  rb_define_singleton_method(rb_mGame, "run",       game_run,       -1);
  rb_define_singleton_method(rb_mGame, "running?",  game_running,   0);
  rb_define_singleton_method(rb_mGame, "terminate", game_terminate, 0);
  rb_define_singleton_method(rb_mGame, "title",     game_title,     0);
  rb_define_singleton_method(rb_mGame, "title=",    game_title_eq,  1);
}
