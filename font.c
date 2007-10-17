#include "starruby.h"

static VALUE rb_cFont;

static VALUE Font_load_path(VALUE self)
{
  VALUE rbLoadPath = rb_iv_get(rb_cFont, "load_path");
  if (rbLoadPath == Qnil) {
    VALUE rbLoadPath = rb_ary_new3(1, rb_str_new2("."));
#ifdef WIN32
    char path[256];
    if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_FONTS, NULL,
                                  SHGFP_TYPE_CURRENT, path))) {
      rb_ary_push(rbLoadPath, rb_str_new2(path));
    } else {
      DWORD errorNo = GetLastError();
      char errorMessage[256];
      snprintf(errorMessage, sizeof(errorMessage), "Win32API error: %d", errorNo);
      rb_raise(rb_eStarRubyError, errorMessage);
    }
#endif
    return rb_iv_set(rb_cFont, "load_path", rbLoadPath);
  } else {
    return rbLoadPath;
  }
}

void InitializeFont(void)
{
  rb_cFont = rb_define_class_under(rb_mStarRuby, "Font", rb_cObject);
  rb_define_singleton_method(rb_cFont, "load_path", Font_load_path, 0);
}
