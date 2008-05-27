#ifndef STARRUBY_H
#define STARRUBY_H

#include <ruby.h>

#ifdef DEFINE_STARRUBY_EXTERN
#define STARRUBY_EXTERN
#else
#define STARRUBY_EXTERN extern
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

void Init_starruby(void);

#endif
