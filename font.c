#include "starruby.h"

#ifdef USE_FONTCONFIG
#include <fontconfig/fontconfig.h>
#endif
#ifdef WIN32
static char windowsFontDirPath[256];
#endif

static VALUE symbol_bold;
static VALUE symbol_italic;
static VALUE symbol_ttc_index;

typedef struct FontFileInfo {
  VALUE rbFontNameSymbol;
  VALUE rbFileNameSymbol;
  int ttcIndex;
  struct FontFileInfo* next;
} FontFileInfo;
static FontFileInfo* fontFileInfos;

static void SearchFont(VALUE rbFilePathOrName,
                       VALUE* rbRealFilePath,
                       int* ttcIndex)
{
  *rbRealFilePath = Qnil;
  if (ttcIndex != NULL)
    *ttcIndex = -1;
  if (rb_funcall(rb_mFileTest, rb_intern("file?"), 1, rbFilePathOrName)) {
    *rbRealFilePath = rbFilePathOrName;
    return;
  }
  VALUE rbFilePathTtf = rb_str_cat2(rb_str_dup(rbFilePathOrName), ".ttf");
  if (rb_funcall(rb_mFileTest, rb_intern("file?"), 1, rbFilePathTtf)) {
    *rbRealFilePath = rbFilePathTtf;
    return;
  }
  VALUE rbFilePathTtc = rb_str_cat2(rb_str_dup(rbFilePathOrName), ".ttc");
  if (rb_funcall(rb_mFileTest, rb_intern("file?"), 1, rbFilePathTtc)) {
    *rbRealFilePath = rbFilePathTtc;
    return;
  }
  VALUE rbFontNameSymbol = ID2SYM(rb_intern(StringValuePtr(rbFilePathOrName)));
  FontFileInfo* info = fontFileInfos;
  while (info) {
    if (info->rbFontNameSymbol == rbFontNameSymbol) {
      *rbRealFilePath = rb_str_new2(rb_id2name(SYM2ID(info->rbFileNameSymbol)));
#ifdef WIN32
      VALUE rbTemp = rb_str_new2(windowsFontDirPath);
      *rbRealFilePath = rb_str_concat(rb_str_cat2(rbTemp, "\\"), *rbRealFilePath);
#endif
      if (ttcIndex != NULL)
        *ttcIndex = info->ttcIndex;
      return;
    }
    info = info->next;
  }
#ifdef USE_FONTCONFIG
  if (!FcInit()) {
    FcFini();
    rb_raise(rb_eStarRubyError, "can't initialize fontconfig library");
    return;
  }
  FcPattern* pattern = FcNameParse((FcChar8*)StringValuePtr(rbFilePathOrName));
  FcObjectSet* objectSet = FcObjectSetBuild(FC_FAMILY, FC_FILE, NULL);
  FcFontSet* fontSet = FcFontList(0, pattern, objectSet);
  if (objectSet)
    FcObjectSetDestroy(objectSet);
  if (pattern)
    FcPatternDestroy(pattern);
  if (fontSet) {
    for (int i = 0; i < fontSet->nfont; i++) {
      FcChar8* fontName = NULL;
      FcChar8* fileName = NULL;
      fontName = FcNameUnparse(fontSet->fonts[i]);
      if (FcPatternGetString(fontSet->fonts[i], FC_FILE, 0, &fileName) !=
	  FcResultMatch)
	continue;
      *rbRealFilePath = rb_str_new2((char*)fileName);
      VALUE rbFontName = rb_str_new2((char*)fontName);
      free(fontName);
      fontName = NULL;
      if (ttcIndex != NULL &&
          strchr(StringValuePtr(rbFontName), ','))
        *ttcIndex = 0;
      break;
    }
    FcFontSetDestroy(fontSet);
  }
  FcFini();
  if (!NIL_P(*rbRealFilePath))
    return;
#endif
  return;
}

static VALUE Font_exist(VALUE self, VALUE rbFilePath)
{
  VALUE rbRealFilePath;
  SearchFont(rbFilePath, &rbRealFilePath, NULL);
  return !NIL_P(rbRealFilePath) ? Qtrue : Qfalse;
}

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

  VALUE rbRealFilePath;
  int preTtcIndex = -1;
  SearchFont(rbPath, &rbRealFilePath, &preTtcIndex);
  if (NIL_P(rbRealFilePath)) {
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
  if (preTtcIndex != -1)
    ttcIndex = preTtcIndex;
  else if (st_lookup(table, symbol_ttc_index, &val))
    ttcIndex = NUM2INT(val);
  
  char* path = StringValuePtr(rbRealFilePath);
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

#define ADD_INFO(currentInfo, _rbFontNameSymbol, _rbFileNameSymbol, _ttcIndex) do {\
  FontFileInfo* info = ALLOC(FontFileInfo);\
  info->rbFontNameSymbol = _rbFontNameSymbol;\
  info->rbFileNameSymbol = _rbFileNameSymbol;\
  info->ttcIndex         = _ttcIndex;\
  info->next             = NULL;\
  currentInfo->next = info;\
  currentInfo = info;\
} while (false)\
                 
void InitializeSdlFont(void)
{
  fontFileInfos = ALLOC(FontFileInfo);
  fontFileInfos->rbFontNameSymbol = Qundef;
  fontFileInfos->rbFileNameSymbol = Qundef;
  fontFileInfos->ttcIndex         = -1;
  fontFileInfos->next             = NULL;
  FontFileInfo* currentInfo = fontFileInfos;
  (void)currentInfo;
  
#ifdef WIN32
  HKEY hKey;
  const char* regPath =
    "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts";
  if (SUCCEEDED(RegOpenKeyEx(HKEY_LOCAL_MACHINE, regPath, 0,
                             KEY_READ, &hKey))) {
    DWORD type;
    char fontNameBuff[256];
    char fileNameBuff[256];
    DWORD fontNameBuffLength;
    DWORD fileNameBuffLength;
    rb_require("nkf");
    VALUE rb_mNKF = rb_const_get(rb_cObject, rb_intern("NKF"));
    VALUE rbNkfOption = rb_str_new2("-S -w --cp932");
    for (DWORD dwIndex = 0; ;dwIndex++) {
      fontNameBuffLength = sizeof(fontNameBuff);
      fileNameBuffLength = sizeof(fileNameBuff);
      LONG result = RegEnumValue(hKey, dwIndex,
                                 fontNameBuff, &fontNameBuffLength,
                                 NULL, &type,
                                 fileNameBuff, &fileNameBuffLength);
      if (result == ERROR_SUCCESS) {
        char* ext = &(fileNameBuff[fileNameBuffLength - 3 - 1]);
        if (tolower(ext[0]) == 't' && tolower(ext[1]) == 't' &&
            (tolower(ext[2]) == 'f' || tolower(ext[2]) == 'c')) {
          char* fontName = fontNameBuff;
          char* fileName = fileNameBuff;
          // Font name must end with ' (TrueType)'.
          fontName[fontNameBuffLength - 11] = '\0'; 
          for (int i = fileNameBuffLength - 1; 0 <= i; i--) {
            if (fileName[i] == '\\') {
              fileName += i + 1;
              break;
            }
          }
          VALUE rbFontName = rb_str_new2(fontName);
          rbFontName = rb_funcall(rb_mNKF, rb_intern("nkf"), 2,
                                  rbNkfOption, rbFontName);
          if (strchr(StringValuePtr(rbFontName), '&')) {
            VALUE rbArr = rb_str_split(rbFontName, "&");
            VALUE* rbFontNames = RARRAY(rbArr)->ptr;
            int arrLength = RARRAY(rbArr)->len;
            int ttcIndex = 0;
            for (int i = 0; i < arrLength; i++) {
              rb_funcall(rbFontNames[i], rb_intern("strip!"), 0);
              if (0 < RSTRING(rbFontNames[i])->len) {
                VALUE rbFontNameSymbol = rb_str_intern(rbFontNames[i]);
                VALUE rbFileNameSymbol = ID2SYM(rb_intern(fileName));
                ADD_INFO(currentInfo, rbFontNameSymbol, rbFileNameSymbol,
                         ttcIndex);
                ttcIndex++;
              }
            }
          } else {
            VALUE rbFontNameSymbol = rb_str_intern(rbFontName);
            VALUE rbFileNameSymbol = ID2SYM(rb_intern(fileName));
            ADD_INFO(currentInfo, rbFontNameSymbol, rbFileNameSymbol, -1);
          }
        }
      } else {
        break;
      }
    }
    RegCloseKey(hKey);
  } else {
    rb_raise(rb_eStarRubyError, "Win32API error: %d", GetLastError());
  }
  if (FAILED(SHGetFolderPath(NULL, CSIDL_FONTS, NULL,
                             SHGFP_TYPE_CURRENT, windowsFontDirPath))) {
    rb_raise(rb_eStarRubyError, "Win32API error: %d", GetLastError());
  }
#endif
}

void FinalizeSdlFont(void)
{
  FontFileInfo* fontFileInfo = fontFileInfos;
  while (fontFileInfo) {
    FontFileInfo* nextFontFileInfo = fontFileInfo->next;
    free(nextFontFileInfo);
    fontFileInfo = nextFontFileInfo;
  }
  fontFileInfo = NULL;
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
