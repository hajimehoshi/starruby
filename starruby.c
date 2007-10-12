#define DEF_GLOBAL
#include "starruby.h"

static SDL_Surface* screen;

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
}

static void TerminateSdl(VALUE dummy)
{
  SDL_FreeSurface(screen);
  SDL_Quit();
}

void Init_starruby(void)
{
  rb_mStarRuby = rb_define_module("StarRuby");

  InitializeColor();
  InitializeGame();
  InitializeScreen();
  InitializeStarRubyError();
  InitializeTexture();

  StartSdl();
  rb_set_end_proc(TerminateSdl, Qnil);
}
