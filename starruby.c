#define DEFINE_STARRUBY_EXTERN
#include "starruby.h"

VALUE
GetCompletePath(VALUE rbPath, bool raiseNotFoundError)
{
  char* path = StringValuePtr(rbPath);
  if (!RTEST(rb_funcall(rb_mFileTest, rb_intern("file?"), 1, rbPath))) {
    volatile VALUE rbPathes = rb_funcall(rb_cDir, rb_intern("[]"), 1,
                                         rb_str_cat2(rb_str_dup(rbPath), ".*"));
    volatile VALUE rbFileName = rb_funcall(rb_cFile, rb_intern("basename"), 1,
                                           rbPath);
    for (int i = 0; i < RARRAY_LEN(rbPathes); i++) {
      volatile VALUE rbFileNameWithoutExt = rb_funcall(rb_cFile,
                                                       rb_intern("basename"), 2,
                                                       RARRAY_PTR(rbPathes)[i],
                                                       rb_str_new2(".*"));
      if (rb_str_cmp(rbFileName, rbFileNameWithoutExt) != 0)
        RARRAY_PTR(rbPathes)[i] = Qnil;
    }
    rb_funcall(rbPathes, rb_intern("compact!"), 0);
    switch (RARRAY_LEN(rbPathes)) {
    case 0:
      if (raiseNotFoundError)
        rb_raise(rb_path2class("Errno::ENOENT"), "%s", path);
      return Qnil;
    case 1:
      return RARRAY_PTR(rbPathes)[0];
    default:
      rb_raise(rb_path2class("ArgumentError"), "ambiguous path: %s", path);
      return Qnil;
    }
  } else {
    return rbPath;
  }
}

void
Init_starruby(void)
{
  if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_JOYSTICK))
    rb_raise_sdl_error();
  InitializeSdlAudio();
  InitializeSdlFont();
  InitializeSdlInput();
  
  rb_mStarRuby = rb_define_module("StarRuby");
  volatile VALUE rbVersion = rb_str_new2("0.1.13");
  OBJ_FREEZE(rbVersion);
  rb_define_const(rb_mStarRuby, "VERSION", rbVersion);
  InitializeAudio();
  InitializeColor();
  InitializeFont();
  InitializeGame();
  InitializeInput();
  InitializeStarRubyError();
  InitializeTexture();
  
#ifdef DEBUG
  TestAffineMatrix();
  TestInput();
#endif
}
