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
#include <SDL_opengl.h>

#include <ruby.h>
#ifndef RHASH_IFNONE
# define RHASH_IFNONE(h) (RHASH(h)->ifnone)
#endif
#ifndef RHASH_TBL
# define RHASH_TBL(h) (RHASH(h)->tbl)
#endif
#ifdef HAVE_RUBY_ST_H
# include "ruby/st.h"
#else
# include "st.h"
#endif

#ifdef WIN32
# include <windows.h>
# include <tchar.h>
# include <winreg.h>
# include <shlobj.h>
# ifndef SHGFP_TYPE_CURRENT
#  define SHGFP_TYPE_CURRENT (0)
# endif
#endif
#include "starruby.h"

#ifndef PI
# ifdef M_PI
#  define PI M_PI
# else
#  define PI (3.1415926535897932384626433832795)
# endif
#endif

typedef struct {
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
  uint8_t blue, green, red, alpha;
#else
  uint8_t alpha, red, green, blue;
#endif
} Color;

typedef union {
  Color color;
  uint32_t value;
} Pixel;

typedef struct {
  uint16_t width, height;
  Pixel* pixels;
  int paletteSize;
  Color* palette;
  uint8_t* indexes;
} Texture;

typedef struct {
  int size;
  TTF_Font* sdlFont;
} Font;

#define MAX(x, y) (((x) >= (y)) ? (x) : (y))
#define MIN(x, y) (((x) <= (y)) ? (x) : (y))
#define DIV255(x) ((x) / 255)

#define rb_raise_sdl_error() \
  rb_raise(strb_GetStarRubyErrorClass(), "%s", SDL_GetError())
#define rb_raise_sdl_mix_error() \
  rb_raise(strb_GetStarRubyErrorClass(), "%s", Mix_GetError())
#define rb_raise_sdl_ttf_error() \
  rb_raise(strb_GetStarRubyErrorClass(), "%s", TTF_GetError())
#define rb_raise_opengl_error() \
  rb_raise(strb_GetStarRubyErrorClass(), "OpenGL Error: 0x%x", glGetError());

VALUE strb_GetColorClass(void);
VALUE strb_GetStarRubyErrorClass(void);
VALUE strb_GetTextureClass(void);

VALUE strb_GetCompletePath(VALUE, bool);

void strb_GetColorFromRubyValue(Color*, VALUE);

void strb_CheckFont(VALUE);
void strb_CheckTexture(VALUE);

VALUE strb_InitializeAudio(VALUE rb_mStarRuby);
VALUE strb_InitializeColor(VALUE rb_mStarRuby);
VALUE strb_InitializeGame(VALUE rb_mStarRuby);
VALUE strb_InitializeFont(VALUE rb_mStarRuby);
VALUE strb_InitializeInput(VALUE rb_mStarRuby);
VALUE strb_InitializeStarRubyError(VALUE rb_mStarRuby);
VALUE strb_InitializeTexture(VALUE rb_mStarRuby);

void strb_UpdateInput(void);

void strb_FinalizeAudio(void);
void strb_FinalizeInput(void);

void strb_InitializeSdlAudio(void);
void strb_InitializeSdlFont(void);
void strb_InitializeSdlInput(void);

void strb_GetRealScreenSize(int*, int*);
void strb_GetScreenSize(int*, int*);
int strb_GetWindowScale(void);

void strb_CheckDisposedTexture(const Texture* const);
bool strb_IsDisposedTexture(const Texture* const);

#ifdef DEBUG
#include <assert.h>
void strb_TestInput(void);
#endif

#ifndef HAVE_RUBY_ENCODING_H
#define rb_intern_str(str) ID2SYM(rb_intern(StringValueCStr(str)))
#endif

#endif
