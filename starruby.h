#ifndef STARRUBY_H
#define STARRUBY_H

#include <stdbool.h>
#include <ruby.h>
#include <SDL.h>
#include <SDL_image.h>

#ifdef DEF_GLOBAL
#define GLOBAL
#else
#define GLOBAL extern
#endif

GLOBAL VALUE rb_mStarRuby;
GLOBAL VALUE rb_eStarRubyError;

#define SCREEN_WIDTH (320)
#define SCREEN_HEIGHT (240)

void InitializeColor(void);
void InitializeGame(void);
void InitializeScreen(void);
void InitializeStarRubyError(void);
void InitializeTexture(void);

#define rb_raise_sdl_error() rb_raise(rb_eStarRubyError, SDL_GetError())

struct Color{
  unsigned char red;
  unsigned char green;
  unsigned char blue;
  unsigned char alpha;
};

#endif
