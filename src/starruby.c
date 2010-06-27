#include "starruby_private.h"

static volatile VALUE rb_eStarRubyError = Qundef;

VALUE
strb_GetCompletePath(VALUE rbPath, bool raiseNotFoundError)
{
  const char* path = StringValueCStr(rbPath);
  if (!RTEST(rb_funcall(rb_mFileTest, rb_intern("file?"), 1, rbPath))) {
    volatile VALUE rbPathes =
      rb_funcall(rb_cDir, rb_intern("[]"), 1,
                 rb_str_cat2(rb_str_dup(rbPath), ".*"));
    volatile VALUE rbFileName =
      rb_funcall(rb_cFile, rb_intern("basename"), 1, rbPath);
    for (int i = 0; i < RARRAY_LEN(rbPathes); i++) {
      volatile VALUE rbFileNameWithoutExt =
        rb_funcall(rb_cFile, rb_intern("basename"), 2,
                   RARRAY_PTR(rbPathes)[i], rb_str_new2(".*"));
      if (rb_str_cmp(rbFileName, rbFileNameWithoutExt) != 0) {
        RARRAY_PTR(rbPathes)[i] = Qnil;
      }
    }
    rb_funcall(rbPathes, rb_intern("compact!"), 0);
    switch (RARRAY_LEN(rbPathes)) {
    case 0:
      if (raiseNotFoundError) {
        rb_raise(rb_path2class("Errno::ENOENT"), "%s", path);
      }
      break;
    case 1:
      return RARRAY_PTR(rbPathes)[0];
    default:
      rb_raise(rb_eArgError, "ambiguous path: %s", path);
      break;
    }
    return Qnil;
  } else {
    return rbPath;
  }
}

static void
FinalizeStarRuby(VALUE unused)
{
  TTF_Quit();
  strb_FinalizeAudio();
  strb_FinalizeInput();
  SDL_Quit();
}

static VALUE
Numeric_degree(VALUE self)
{
  return rb_float_new(NUM2DBL(self) * PI / 180.0);
}

VALUE
strb_GetStarRubyErrorClass(void)
{
  return rb_eStarRubyError;
}

void
Init_starruby(void)
{
  volatile VALUE rb_mStarRuby = rb_define_module("StarRuby");
  rb_eStarRubyError =
    rb_define_class_under(rb_mStarRuby, "StarRubyError", rb_eStandardError);

  if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_JOYSTICK)) {
    rb_raise_sdl_error();
  }
  strb_InitializeSdlAudio();
  strb_InitializeSdlFont();
  strb_InitializeSdlInput();

  volatile VALUE rbVersion = rb_str_new2("0.4.0");
  OBJ_FREEZE(rbVersion);
  rb_define_const(rb_mStarRuby, "VERSION", rbVersion);
  strb_InitializeAudio(rb_mStarRuby);
  strb_InitializeColor(rb_mStarRuby);
  strb_InitializeFont(rb_mStarRuby);
  strb_InitializeGame(rb_mStarRuby);
  strb_InitializeInput(rb_mStarRuby);
  strb_InitializeTexture(rb_mStarRuby);

  rb_set_end_proc(FinalizeStarRuby, Qnil);

  rb_define_method(rb_cNumeric, "degree",  Numeric_degree, 0);
  rb_define_method(rb_cNumeric, "degrees", Numeric_degree, 0);

#ifdef DEBUG
  strb_TestInput();
#endif
}
