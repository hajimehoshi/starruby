#define DEFINE_STARRUBY_EXTERN
#include "starruby.h"

static bool isSdlQuitted = false;
bool IsSdlQuitted()
{
  return isSdlQuitted;
}

static void InitializeSdl(void)
{
  Uint32 flags = SDL_INIT_VIDEO | SDL_INIT_JOYSTICK |
    SDL_INIT_AUDIO | SDL_INIT_TIMER;
  if (SDL_Init(flags) < 0)
    rb_raise_sdl_error();
  
  SDL_ShowCursor(SDL_DISABLE);

  InitializeSdlGame();
  InitializeSdlInput();
  if (TTF_Init())
    rb_raise_sdl_ttf_error();
}

static void FinalizeSdl(VALUE unused)
{
  TTF_Quit();
  FinalizeSdlInput();
  FinalizeSdlGame();
  SDL_Quit();
  isSdlQuitted = true;
}

void Init_starruby(void)
{
  InitializeSdl();
  rb_set_end_proc(FinalizeSdl, Qnil);
  
  rb_mStarRuby = rb_define_module("StarRuby");

  InitializeColor();
  InitializeFont();
  InitializeGame();
  InitializeInput();
  InitializeStarRubyError();
  InitializeTexture();
  
#ifdef DEBUG
  AffineMatrix_Test();
#endif
}
