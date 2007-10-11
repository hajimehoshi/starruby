#ifndef STARRUBY_H
#define STARRUBY_H

#include <stdbool.h>
#include <ruby.h>
#include <SDL.h>
#include <SDL_image.h>

extern VALUE rb_mStarRuby;

void InitializeGame(void);
void InitializeScreen(void);
void InitializeStarRubyError(void);
void InitializeTexture(void);

inline void rb_raise_star_ruby_error(char*);
inline void rb_raise_sdl_error(void);

#endif
