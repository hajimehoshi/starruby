#define DEFINE_STARRUBY_EXTERN
#include "starruby.h"

static SDL_Surface* screen;

static bool sdlIsQuitted = false;
bool SdlIsQuitted()
{
  return sdlIsQuitted;
}

static void StartSdl(void)
{
  Uint32 flags = SDL_INIT_VIDEO | SDL_INIT_JOYSTICK |
    SDL_INIT_AUDIO | SDL_INIT_TIMER;
  if (SDL_Init(flags) < 0)
    rb_raise_sdl_error();
  
  SDL_ShowCursor(SDL_DISABLE);

  Uint32 options = SDL_HWACCEL | SDL_DOUBLEBUF;

  screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 32, options);
  if (screen == NULL)
    rb_raise_sdl_error();

  if (TTF_Init())
    rb_raise_sdl_ttf_error()
}

static void TerminateSdl(VALUE unused)
{
  TTF_Quit();
  SDL_FreeSurface(screen);
  SDL_Quit();
  sdlIsQuitted = true;
}

void Init_starruby(void)
{
  StartSdl();
  rb_set_end_proc(TerminateSdl, Qnil);
  
  rb_mStarRuby = rb_define_module("StarRuby");

  InitializeColor();
  InitializeFont();
  InitializeGame(screen);
  InitializeInput();
  InitializeScreen();
  InitializeStarRubyError();
  InitializeTexture();
  
#ifdef DEBUG
  AffineMatrix_Test();
#endif
}
