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

#ifdef DEFINE_STARRUBY_EXTERN
#define STARRUBY_EXTERN
#else
#define STARRUBY_EXTERN extern
#endif

#ifndef PI
#define PI (3.1415926535897932384626433832795)
#endif

// TODO: Change names
STARRUBY_EXTERN VALUE rb_mAudio;
STARRUBY_EXTERN VALUE rb_cColor;
STARRUBY_EXTERN VALUE rb_cFont;
STARRUBY_EXTERN VALUE rb_mGame;
STARRUBY_EXTERN VALUE rb_mInput;
STARRUBY_EXTERN VALUE rb_mStarRuby;
STARRUBY_EXTERN VALUE rb_eStarRubyError;
STARRUBY_EXTERN VALUE rb_cTexture;
STARRUBY_EXTERN VALUE rb_cTone;

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

#define rb_raise_sdl_error() rb_raise(rb_eStarRubyError, "%s", SDL_GetError())
#define rb_raise_sdl_mix_error()\
  rb_raise(rb_eStarRubyError, "%s", Mix_GetError())
#define rb_raise_sdl_ttf_error()\
  rb_raise(rb_eStarRubyError, "%s", TTF_GetError())

VALUE strb_GetCompletePath(VALUE, bool);

void strb_InitializeAudio(void);
void strb_InitializeColor(void);
void strb_InitializeGame(void);
void strb_InitializeFont(void);
void strb_InitializeInput(void);
void strb_InitializeStarRubyError(void);
void strb_InitializeTexture(void);

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
