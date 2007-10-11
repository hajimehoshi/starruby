#ifndef STARRUBY_H
#define STARRUBY_H

#include <ruby.h>
#include <SDL.h>

extern VALUE rb_mStarRuby;
extern VALUE rb_eStarRubyError;

void init_game(void);

#endif
