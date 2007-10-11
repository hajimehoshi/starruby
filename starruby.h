#ifndef STARRUBY_H
#define STARRUBY_H

#include <stdbool.h>
#include <ruby.h>
#include <SDL.h>

extern VALUE rb_mStarRuby;
extern VALUE rb_eStarRubyError;

void init_game(void);

inline void rb_raise_sdl_error(void);

#endif
