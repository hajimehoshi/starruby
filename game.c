#include "starruby.h"

static VALUE game_run(int argc, VALUE* argv, VALUE self)
{
  VALUE block;
  rb_scan_args(argc, argv, "0&", &block);

  Uint32 flags = SDL_INIT_VIDEO | SDL_INIT_JOYSTICK |
    SDL_INIT_AUDIO | SDL_INIT_TIMER;
  if (SDL_Init(flags) < 0)
    rb_raise(rb_eStarRubyError, SDL_GetError());
  
  SDL_ShowCursor(SDL_DISABLE);
  
  rb_yield(Qnil);
  
  SDL_Quit();
  
  return Qnil;
}

void init_game(void)
{
  VALUE rb_mGame = rb_define_module_under(rb_mStarRuby, "Game");
  rb_define_singleton_method(rb_mGame, "run", game_run, -1);
}
