#ifndef STARRUBY_H
#define STARRUBY_H

#include <stdbool.h>
#include <ruby.h>
#include <SDL.h>
#include <SDL_image.h>

extern VALUE rb_mStarRuby;
extern VALUE rb_eStarRubyError;

void InitializeGame(void);
void InitializeScreen(void);
void InitializeStarRubyError(void);
void InitializeTexture(void);

#define rb_raise_star_ruby_error(message) rb_raise(rb_eStarRubyError, message)
#define rb_raise_sdl_error() rb_raise(rb_eStarRubyError, SDL_GetError())

#endif
