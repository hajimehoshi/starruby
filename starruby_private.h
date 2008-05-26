#ifndef STARRUBY_PRIVATE_H
#define STARRUBY_PRIVATE_H

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

#ifndef PI
#define PI (3.1415926535897932384626433832795)
#endif

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
