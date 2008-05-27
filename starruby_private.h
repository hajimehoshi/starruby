#ifndef STARRUBY_PRIVATE_H
#define STARRUBY_PRIVATE_H

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <ruby.h>
#ifdef WIN32
#include <windows.h>
#include <winreg.h>
#include <shlobj.h>
#ifndef SHGFP_TYPE_CURRENT
#define SHGFP_TYPE_CURRENT (0)
#endif
#endif
#include "starruby.h"

#ifndef PI
#define PI (3.1415926535897932384626433832795)
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

#define MAX(x, y) ((x >= y) ? x : y)
#define MIN(x, y) ((x <= y) ? x : y)
#define DIV255(x) ((x + 255) >> 8)

#define rb_raise_sdl_error() rb_raise(strb_GetStarRubyError(), "%s", SDL_GetError())
#define rb_raise_sdl_mix_error() \
  rb_raise(strb_GetStarRubyError(), "%s", Mix_GetError())
#define rb_raise_sdl_ttf_error() \
  rb_raise(strb_GetStarRubyError(), "%s", TTF_GetError())

#define strb_GetColorFromRubyValue(colorP, rbColor) \
  do {                                              \
    Check_Type(rbColor, T_DATA);                    \
    Pixel p;                                        \
    p.value = (uint32_t)DATA_PTR(rbColor);          \
    *colorP = p.color;                              \
  } while (false)

VALUE strb_GetStarRubyError(void);

VALUE strb_GetCompletePath(VALUE, bool);

VALUE strb_InitializeAudio(VALUE rb_mStarRuby);
VALUE strb_InitializeColor(VALUE rb_mStarRuby);
VALUE strb_InitializeGame(VALUE rb_mStarRuby);
VALUE strb_InitializeFont(VALUE rb_mStarRuby);
VALUE strb_InitializeInput(VALUE rb_mStarRuby);
VALUE strb_InitializeStarRubyError(VALUE rb_mStarRuby);
VALUE strb_InitializeTexture(VALUE rb_mStarRuby, VALUE rb_cColor);

void strb_UpdateAudio(void);
void strb_UpdateInput(void);

void strb_FinalizeAudio(void);
void strb_FinalizeInput(void);

void strb_InitializeSdlAudio(void);
void strb_InitializeSdlFont(void);
void strb_InitializeSdlInput(void);

int strb_GetWindowScale(void);

#ifdef DEBUG
#include <assert.h>
void strb_TestInput(void);
#endif

#endif
