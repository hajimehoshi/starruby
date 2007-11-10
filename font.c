#include "starruby.h"

static VALUE symbol_bold;
static VALUE symbol_italic;
static VALUE symbol_ttc_index;

static VALUE SearchFontPath(VALUE rbFilePath)
{
  if (rb_funcall(rb_mFileTest, rb_intern("file?"), 1, rbFilePath))
    return rbFilePath;
  VALUE rbFilePathTtf = rb_str_cat2(rb_str_dup(rbFilePath), ".ttf");
  if (rb_funcall(rb_mFileTest, rb_intern("file?"), 1, rbFilePathTtf))
    return rbFilePathTtf;
  VALUE rbFilePathTtc = rb_str_cat2(rb_str_dup(rbFilePath), ".ttc");
  if (rb_funcall(rb_mFileTest, rb_intern("file?"), 1, rbFilePathTtc))
    return rbFilePathTtc;
#ifdef WIN32
  HKEY hKey;
  char* regPath = "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts";
  if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, regPath, 0, KEY_READ, &hKey)
      == ERROR_SUCCESS) {
    DWORD type;
    char valueName[256];
    char data[256];
    for (DWORD dwIndex = 0; ;dwIndex++) {
      DWORD valueNameLength = sizeof(valueName);
      DWORD dataLength = sizeof(data);
      LONG result = RegEnumValue(hKey, dwIndex, valueName, &valueNameLength,
                                 NULL, &type, data, &dataLength);
      if (result == ERROR_SUCCESS) {
        printf("%s\n", valueName);
      } else {
        break;
      }
    }
    /*if (SUCCEEDED(RegQueryValueEx(hKey, "Arial (TrueType)", NULL, &dataType,
                                  fontFileName, &fontFileNameLength))) {
      printf("%s\n", fontFileName);
      printf("%d\n", (int)fontFileNameLength);
      if (dataType != REG_SZ)
        return Qnil;
    }*/
    RegCloseKey(hKey);
  } else {
    rb_raise(rb_eStarRubyError, "Win32API error: %d", GetLastError());
  }
#endif
  return Qnil;
}

static VALUE Font_exist(VALUE self, VALUE rbFilePath)
{
  return !NIL_P(SearchFontPath(rbFilePath)) ? Qtrue : Qfalse;
}

/*static VALUE Font_load_path(VALUE self)
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
  return rb_ary_new();
}*/

static void Font_free(Font* font)
{
  if (!IsSdlQuitted()) {
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

static VALUE Font_initialize(int argc, VALUE* argv, VALUE self)
{
  VALUE rbPath;
  VALUE rbSize;
  VALUE rbOptions;
  rb_scan_args(argc, argv, "21", &rbPath, &rbSize, &rbOptions);
  if (NIL_P(rbOptions))
    rbOptions = rb_hash_new();
  
  VALUE rbFullPath = SearchFontPath(rbPath);
  if (NIL_P(rbFullPath)) {
    char* path = StringValuePtr(rbPath);
    rb_raise(rb_path2class("Errno::ENOENT"), "%s", path);
    return Qnil;
  }

  bool bold = false;
  bool italic = false;
  int ttcIndex = 0;

  VALUE val;
  st_table* table = RHASH(rbOptions)->tbl;
  if (st_lookup(table, symbol_bold, &val))
    bold = RTEST(val);
  if (st_lookup(table, symbol_italic, &val))
    italic = RTEST(val);
  if (st_lookup(table, symbol_ttc_index, &val))
    ttcIndex = NUM2INT(val);
  
  char* path = StringValuePtr(rbFullPath);
  int size = NUM2INT(rbSize);
  
  Font* font;
  Data_Get_Struct(self, Font, font);
  font->size = size;
  font->sdlFont = TTF_OpenFontIndex(path, size, ttcIndex);
  if (!font->sdlFont) {
    rb_raise_sdl_ttf_error();
    return Qnil;
  }
  int style = TTF_STYLE_NORMAL |
    (bold ? TTF_STYLE_BOLD : 0) | (italic ? TTF_STYLE_ITALIC : 0);
  TTF_SetFontStyle(font->sdlFont, style);
  
  return Qnil;
}

static VALUE Font_bold(VALUE self)
{
  Font* font;
  Data_Get_Struct(self, Font, font);
  if (!font->sdlFont) {
    rb_raise(rb_eTypeError, "can't use disposed font");
    return Qnil;
  }
  return (TTF_GetFontStyle(font->sdlFont) & TTF_STYLE_BOLD) ? Qtrue : Qfalse;
}

static VALUE Font_dispose(VALUE self)
{
  Font* font;
  Data_Get_Struct(self, Font, font);
  TTF_CloseFont(font->sdlFont);
  font->sdlFont = NULL;
  return Qnil;
}

static VALUE Font_disposed(VALUE self)
{
  Font* font;
  Data_Get_Struct(self, Font, font);
  return !(font->sdlFont) ? Qtrue : Qfalse;
}

static VALUE Font_italic(VALUE self)
{
  Font* font;
  Data_Get_Struct(self, Font, font);
  if (!font->sdlFont) {
    rb_raise(rb_eTypeError, "can't use disposed font");
    return Qnil;
  }
  return (TTF_GetFontStyle(font->sdlFont) & TTF_STYLE_ITALIC) ? Qtrue : Qfalse;
}

static VALUE Font_get_size(VALUE self, VALUE rbText)
{
  Font* font;
  Data_Get_Struct(self, Font, font);
  if (!font->sdlFont) {
    rb_raise(rb_eTypeError, "can't use disposed font");
    return Qnil;
  }
  char* text = StringValuePtr(rbText);
  int width, height;
  if (TTF_SizeUTF8(font->sdlFont, text, &width, &height)) {
    rb_raise_sdl_ttf_error();
    return Qnil;
  }
  VALUE rbSize = rb_assoc_new(INT2NUM(width), INT2NUM(height));
  OBJ_FREEZE(rbSize);
  return rbSize;
}

static VALUE Font_name(VALUE self)
{
  Font* font;
  Data_Get_Struct(self, Font, font);
  if (!font->sdlFont) {
    rb_raise(rb_eTypeError, "can't use disposed font");
    return Qnil;
  }
  return rb_str_new2(TTF_FontFaceFamilyName(font->sdlFont));
}

static VALUE Font_size(VALUE self)
{
  Font* font;
  Data_Get_Struct(self, Font, font);
  if (!font->sdlFont) {
    rb_raise(rb_eTypeError, "can't use disposed font");
    return Qnil;
  }
  return INT2NUM(font->size);
}

void InitializeFont(void)
{
  rb_cFont = rb_define_class_under(rb_mStarRuby, "Font", rb_cObject);
  rb_define_singleton_method(rb_cFont, "exist?",    Font_exist,     1);
  rb_define_alloc_func(rb_cFont, Font_alloc);
  rb_define_private_method(rb_cFont, "initialize", Font_initialize, -1);
  rb_define_method(rb_cFont, "bold?",     Font_bold,     0);
  rb_define_method(rb_cFont, "dispose",   Font_dispose,  0);
  rb_define_method(rb_cFont, "disposed?", Font_disposed, 0);
  rb_define_method(rb_cFont, "get_size",  Font_get_size, 1);
  rb_define_method(rb_cFont, "italic?",   Font_italic,   0);
  rb_define_method(rb_cFont, "name",      Font_name,     0);
  rb_define_method(rb_cFont, "size",      Font_size,     0);

  symbol_bold      = ID2SYM(rb_intern("bold"));
  symbol_italic    = ID2SYM(rb_intern("italic"));
  symbol_ttc_index = ID2SYM(rb_intern("ttc_index"));
}
