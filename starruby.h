#ifndef STARRUBY_H
#define STARRUBY_H

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <ruby.h>
#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#ifdef WIN32
#include <windows.h>
#include <winreg.h>
#include <shlobj.h>
#ifndef SHGFP_TYPE_CURRENT
#define SHGFP_TYPE_CURRENT (0)
#endif
#endif

#ifdef DEFINE_STARRUBY_EXTERN
#define STARRUBY_EXTERN
#else
#define STARRUBY_EXTERN extern
#endif

typedef struct {
  uint8_t blue, green, red, alpha;
} Color;

typedef union {
  Color color;
  uint32_t value;
} Pixel;

typedef struct {
  uint16_t width, height;
  Pixel* pixels;
} Texture;

typedef struct {
  int size;
  TTF_Font* sdlFont;
} Font;

STARRUBY_EXTERN VALUE rb_mAudio;
STARRUBY_EXTERN VALUE rb_cColor;
STARRUBY_EXTERN VALUE rb_cFont;
STARRUBY_EXTERN VALUE rb_mGame;
STARRUBY_EXTERN VALUE rb_mInput;
STARRUBY_EXTERN VALUE rb_mStarRuby;
STARRUBY_EXTERN VALUE rb_eStarRubyError;
STARRUBY_EXTERN VALUE rb_cTexture;
STARRUBY_EXTERN VALUE rb_cTone;

void Init_starruby(void);

#endif
