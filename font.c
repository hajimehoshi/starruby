#include "starruby.h"

static VALUE rb_cFont;

static VALUE Font_load_path(VALUE);
static VALUE SearchFont(VALUE rbFilePath)
{
  VALUE rbLoadPath = Font_load_path(rb_cFont);
  int len = RARRAY(rbLoadPath)->len;
  VALUE* ptr = RARRAY(rbLoadPath)->ptr;
  for (int i = 0; i < len; i++, ptr++) {
    VALUE rbFullPath = rb_funcall(rb_cFile, rb_intern("join"), 2,
                                  *ptr, rbFilePath);
#ifdef WIN32
    // if a path began with 'C:', the path separator must be the backslash.
    rb_funcall(rbFullPath, rb_intern("gsub!"), 2,
               rb_str_new2("/"), rb_str_new2("\\"));
#endif
    if (rb_funcall(rb_mFileTest, rb_intern("file?"), 1, rbFullPath))
      return rbFullPath;
    VALUE rbFullPathTtf = rb_str_cat2(rb_str_dup(rbFullPath), ".ttf");
    if (rb_funcall(rb_mFileTest, rb_intern("file?"), 1, rbFullPathTtf))
      return rbFullPathTtf;
    VALUE rbFullPathTtc = rb_str_cat2(rb_str_dup(rbFullPath), ".ttc");
    if (rb_funcall(rb_mFileTest, rb_intern("file?"), 1, rbFullPathTtc))
      return rbFullPathTtc;
  }
  return Qnil;
}

static VALUE Font_exist(VALUE self, VALUE rbFilePath)
{
  return !NIL_P(SearchFont(rbFilePath)) ? Qtrue : Qfalse;
}

static VALUE Font_load_path(VALUE self)
{
  VALUE rbLoadPath = rb_iv_get(self, "load_path");
  if (NIL_P(rbLoadPath)) {
    VALUE rbLoadPath = rb_ary_new3(1, rb_str_new2("."));
#ifdef WIN32
    char path[256];
    if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_FONTS, NULL,
                                  SHGFP_TYPE_CURRENT, path))) {
      rb_ary_push(rbLoadPath, rb_str_new2(path));
    } else {
      rb_raise(rb_eStarRubyError, "Win32API error: %d", GetLastError());
    }
#endif
    return rb_iv_set(self, "load_path", rbLoadPath);
  } else {
    return rbLoadPath;
  }
}

static void Font_free(Font* font)
{
  if (!SdlIsQuitted()) {
    TTF_CloseFont(font->sdlFont);
    font->sdlFont = NULL;
  }
  free(font);
}

static VALUE Font_alloc(VALUE klass)
{
  Font* font = ALLOC(Font);
  font->sdlFont = NULL;
  return Data_Wrap_Struct(klass, 0, Font_free, font);
}

static VALUE Font_initialize(VALUE self, VALUE rbPath, VALUE rbSize)
{
  VALUE rbFullPath = SearchFont(rbPath);
  if (NIL_P(rbFullPath)) {
    char* path = StringValuePtr(rbPath);
    rb_raise(rb_path2class("Errno::ENOENT"), "%s", path);
    return Qnil;
  }
  
  char* path = StringValuePtr(rbFullPath);
  int size = NUM2INT(rbSize);
  
  Font* font;
  Data_Get_Struct(self, Font, font);
  font->sdlFont = TTF_OpenFont(path, size);
  if (!font->sdlFont) {
    rb_raise_sdl_ttf_error();
    return Qnil;
  }
  return Qnil;
}

static VALUE Font_bold(VALUE self)
{
  return Qfalse;
}

static VALUE Font_italic(VALUE self)
{
  return Qfalse;
}

static VALUE Font_name(VALUE self)
{
  Font* font;
  Data_Get_Struct(self, Font, font);
  if (!font) {
    rb_raise(rb_eTypeError, "can't use disposed font");
    return Qnil;
  }
  return rb_str_new2(TTF_FontFaceFamilyName(font->sdlFont));
  return Qnil;
}

static VALUE Font_size(VALUE self)
{
  return INT2NUM(12);
}

void InitializeFont(void)
{
  rb_cFont = rb_define_class_under(rb_mStarRuby, "Font", rb_cObject);
  rb_define_singleton_method(rb_cFont, "exist?",    Font_exist,     1);
  rb_define_singleton_method(rb_cFont, "load_path", Font_load_path, 0);
  rb_define_alloc_func(rb_cFont, Font_alloc);
  rb_define_private_method(rb_cFont, "initialize", Font_initialize, 2);
  rb_define_method(rb_cFont, "bold?",   Font_bold,   0);
  rb_define_method(rb_cFont, "italic?", Font_italic, 0);
  rb_define_method(rb_cFont, "name",    Font_name,   0);
  rb_define_method(rb_cFont, "size",    Font_size,   0);
}
