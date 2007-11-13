#define DEFINE_STARRUBY_EXTERN
#include "starruby.h"

VALUE GetCompletePath(VALUE rbPath)
{
  char* path = StringValuePtr(rbPath);
  
  if (!RTEST(rb_funcall(rb_mFileTest, rb_intern("file?"), 1, rbPath))) {
    VALUE rbPathes = rb_funcall(rb_cDir, rb_intern("[]"), 1,
                                rb_str_cat2(rb_str_dup(rbPath), ".*"));
    struct RArray* arrPathes = RARRAY(rbPathes);
    int len = arrPathes->len;
    VALUE rbFileName = rb_funcall(rb_cFile, rb_intern("basename"), 1, rbPath);
    for (int i = 0; i < len; i++) {
      VALUE rbFileNameWithoutExt = rb_funcall(rb_cFile, rb_intern("basename"), 2,
                                              arrPathes->ptr[i],
                                              rb_str_new2(".*"));
      if (rb_str_cmp(rbFileName, rbFileNameWithoutExt) != 0)
        arrPathes->ptr[i] = Qnil;
    }
    rb_funcall(rbPathes, rb_intern("compact!"), 0);
    switch (arrPathes->len) {
    case 0:
      rb_raise(rb_path2class("Errno::ENOENT"), "%s", path);
      return Qnil;
    case 1:
      return arrPathes->ptr[0];
    default:
      rb_raise(rb_path2class("ArgumentError"), "ambiguous path: %s", path);
      return Qnil;
    }
  } else {
    return rbPath;
  }
}

static bool isSdlQuitted = false;
bool IsSdlQuitted()
{
  return isSdlQuitted;
}

static void InitializeSdl(void)
{
  Uint32 flags = SDL_INIT_VIDEO | SDL_INIT_JOYSTICK |
    SDL_INIT_AUDIO | SDL_INIT_TIMER;
  if (SDL_Init(flags) < 0)
    rb_raise_sdl_error();
  
  SDL_ShowCursor(SDL_DISABLE);

  InitializeSdlAudio();
  InitializeSdlFont();
  InitializeSdlInput();
  if (TTF_Init())
    rb_raise_sdl_ttf_error();
}

static void FinalizeSdl(VALUE unused)
{
  TTF_Quit();
  FinalizeSdlInput();
  FinalizeSdlFont();
  FinalizeSdlAudio();
  SDL_Quit();
  isSdlQuitted = true;
}

void Init_starruby(void)
{
  rb_mStarRuby = rb_define_module("StarRuby");

  InitializeAudio();
  InitializeColor();
  InitializeFont();
  InitializeGame();
  InitializeInput();
  InitializeStarRubyError();
  InitializeTexture();

  InitializeSdl();
  rb_set_end_proc(FinalizeSdl, Qnil);
  
#ifdef DEBUG
  TestAffineMatrix();
  TestInput();
#endif
}
