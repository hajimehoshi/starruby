#ifndef STARRUBY_H
#define STARRUBY_H

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <ruby.h>
#include <SDL.h>
#include <SDL_image.h>

#ifdef DEFINE_STARRUBY_EXTERN
#define STARRUBY_EXTERN
#else
#define STARRUBY_EXTERN extern
#endif

typedef struct {
  uint8_t blue;
  uint8_t green;
  uint8_t red;
  uint8_t alpha;
} Color;

typedef union {
  Color color;
  uint32_t value;
} Pixel;

typedef struct {
  uint16_t width;
  uint16_t height;
  Pixel* pixels;
} Texture;

typedef struct {
  double a;
  double b;
  double c;
  double d;
  double tx;
  double ty;
} AffineMatrix;

#ifndef PI
#define PI (3.1415926535897932384626433832795)
#endif
#define SCREEN_WIDTH (320)
#define SCREEN_HEIGHT (240)

STARRUBY_EXTERN VALUE rb_cColor;
STARRUBY_EXTERN VALUE rb_mGame;
STARRUBY_EXTERN VALUE rb_mScreen;
STARRUBY_EXTERN VALUE rb_mStarRuby;
STARRUBY_EXTERN VALUE rb_eStarRubyError;
STARRUBY_EXTERN VALUE rb_cTexture;
STARRUBY_EXTERN VALUE rb_cTone;

#define rb_raise_sdl_error() rb_raise(rb_eStarRubyError, SDL_GetError())

#define MAX(x, y) ((x >= y) ? x : y)
#define MIN(x, y) ((x <= y) ? x : y)
#define NORMALIZE(x, min, max) ((x < min) ? min : ((max < x) ? max : x))

void Init_starruby(void);

void InitializeColor(void);
void InitializeGame(SDL_Surface*);
void InitializeScreen(void);
void InitializeStarRubyError(void);
void InitializeTexture(void);

void UpdateScreen(SDL_Surface*);

void AffineMatrix_Concat(AffineMatrix*, AffineMatrix*);
void AffineMatrix_Invert(AffineMatrix*);
bool AffineMatrix_IsRegular(AffineMatrix*);
void AffineMatrix_Transform(AffineMatrix*, double*, double*);

#ifdef DEBUG
#include <assert.h>
void AffineMatrix_Test(void);
#endif

#endif
