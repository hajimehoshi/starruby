#ifndef STARRUBY_H
#define STARRUBY_H

#include <stdbool.h>
#include <stdint.h>
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

void Init_starruby(void);

void InitializeColor(void);
void InitializeGame(void);
void InitializeScreen(void);
void InitializeStarRubyError(void);
void InitializeTexture(void);
void InitializeTone(void);

#define rb_raise_sdl_error() rb_raise(rb_eStarRubyError, SDL_GetError())

#define NORMALIZE(x, min, max) ((x < min) ? min : ((max < x) ? max : x))

struct Color {
  uint8_t alpha;
  uint8_t red;
  uint8_t blue;
  uint8_t green;
};

struct Tone {
  int16_t red;
  int16_t green;
  int16_t blue;
  uint8_t saturation;
};

struct Texture {
  uint16_t width;
  uint16_t height;
  uint64_t* pixels;
};

#endif
