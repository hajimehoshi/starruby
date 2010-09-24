#include "starruby_private.h"

#ifdef HAVE_FONTCONFIG_FONTCONFIG_H
#include <fontconfig/fontconfig.h>
#endif
#ifdef WIN32
static volatile VALUE rbWindowsFontDirPathSymbol = Qundef;
#endif

static VALUE rbFontCache = Qundef;

static volatile VALUE symbol_bold      = Qundef;
static volatile VALUE symbol_italic    = Qundef;
static volatile VALUE symbol_ttc_index = Qundef;

typedef struct FontFileInfo {
  VALUE rbFontNameSymbol;
  VALUE rbFileNameSymbol;
  int ttcIndex;
  struct FontFileInfo* next;
} FontFileInfo;
static FontFileInfo* fontFileInfos;

static void Font_free(Font*);
inline void
strb_CheckFont(VALUE rbFont)
{
  Check_Type(rbFont, T_DATA);
  if (RDATA(rbFont)->dfree != (RUBY_DATA_FUNC)Font_free) {
    rb_raise(rb_eTypeError, "wrong argument type %s (expected StarRuby::Font)",
             rb_obj_classname(rbFont));
  }
}

static void
SearchFont(VALUE rbFilePathOrName,
           VALUE* volatile rbRealFilePath, int* ttcIndex)
{
  *rbRealFilePath = Qnil;
  if (ttcIndex != NULL) {
    *ttcIndex = -1;
  }
  *rbRealFilePath = strb_GetCompletePath(rbFilePathOrName, false);
  if (!NIL_P(*rbRealFilePath)) {
    return;
  }
  volatile VALUE rbFontNameSymbol =
    ID2SYM(rb_intern_str(rbFilePathOrName));
  FontFileInfo* info = fontFileInfos;
  while (info) {
    if (info->rbFontNameSymbol == rbFontNameSymbol) {
      *rbRealFilePath = rb_str_new2(rb_id2name(SYM2ID(info->rbFileNameSymbol)));
#ifdef WIN32
      volatile VALUE rbTemp =
        rb_str_new2(rb_id2name(SYM2ID(rbWindowsFontDirPathSymbol)));
      *rbRealFilePath = rb_str_concat(rb_str_cat2(rbTemp, "\\"), *rbRealFilePath);
#endif
      if (ttcIndex != NULL) {
        *ttcIndex = info->ttcIndex;
      }
      return;
    }
    info = info->next;
  }
#ifdef HAVE_FONTCONFIG_FONTCONFIG_H
  if (!FcInit()) {
    FcFini();
    rb_raise(strb_GetStarRubyErrorClass(), "can't initialize fontconfig library");
    return;
  }
  int nameLength = RSTRING_LEN(rbFilePathOrName) + 1;
  char name[nameLength];
  strncpy(name, StringValueCStr(rbFilePathOrName), nameLength);
  char* delimiter = strchr(name, ',');
  char* style = NULL;
  if (delimiter) {
    *delimiter = '\0';
    style = delimiter + 1;
    char* nameTail = delimiter - 1;
    while (*nameTail == ' ') {
      *nameTail = '\0';
      nameTail--;
    }
    while (*style == ' ') {
      style++;
    }
  }
  FcPattern* pattern = FcPatternBuild(NULL, FC_FAMILY, FcTypeString, name, NULL);
  if (style && 0 < strlen(style)) {
    FcPatternAddString(pattern, FC_STYLE, (FcChar8*)style);
  }
  FcObjectSet* objectSet = FcObjectSetBuild(FC_FAMILY, FC_FILE, NULL);
  FcFontSet* fontSet = FcFontList(NULL, pattern, objectSet);
  if (objectSet) {
    FcObjectSetDestroy(objectSet);
  }
  if (pattern) {
    FcPatternDestroy(pattern);
  }
  if (fontSet) {
    for (int i = 0; i < fontSet->nfont; i++) {
      FcChar8* fileName = NULL;
      if (FcPatternGetString(fontSet->fonts[i], FC_FILE, 0, &fileName) ==
          FcResultMatch) {
        FcChar8* fontName = FcNameUnparse(fontSet->fonts[i]);
        *rbRealFilePath = rb_str_new2((char*)fileName);
        volatile VALUE rbFontName = rb_str_new2((char*)fontName);
        free(fontName);
        fontName = NULL;
        if (ttcIndex != NULL && strchr(StringValueCStr(rbFontName), ',')) {
          *ttcIndex = 0;
        }
      }
    }
    FcFontSetDestroy(fontSet);
  }
  FcFini();
  if (!NIL_P(*rbRealFilePath)) {
    return;
  }
#endif
  return;
}

static VALUE
Font_s_exist(VALUE self, VALUE rbFilePath)
{
  volatile VALUE rbRealFilePath = Qnil;
  SearchFont(rbFilePath, (VALUE*)&rbRealFilePath, NULL);
  return !NIL_P(rbRealFilePath) ? Qtrue : Qfalse;
}

static void
Font_free(Font* font)
{
  if (TTF_WasInit()) {
    TTF_CloseFont(font->sdlFont);
  }
  font->sdlFont = NULL;
  free(font);
}

static VALUE
Font_s_new(int argc, VALUE* argv, VALUE self)
{
  volatile VALUE rbPath, rbSize, rbOptions;
  rb_scan_args(argc, argv, "21", &rbPath, &rbSize, &rbOptions);
  if (NIL_P(rbOptions)) {
    rbOptions = rb_hash_new();
  }
  volatile VALUE rbRealFilePath;
  int preTtcIndex = -1;
  SearchFont(rbPath, (VALUE*)&rbRealFilePath, &preTtcIndex);
  if (NIL_P(rbRealFilePath)) {
    char* path = StringValueCStr(rbPath);
    rb_raise(rb_path2class("Errno::ENOENT"), "%s", path);
    return Qnil;
  }
  int size = NUM2INT(rbSize);
  bool bold = false;
  bool italic = false;
  int ttcIndex = 0;
  volatile VALUE val;
  Check_Type(rbOptions, T_HASH);
  if (!NIL_P(val = rb_hash_aref(rbOptions, symbol_bold))) {
    bold = RTEST(val);
  }
  if (!NIL_P(val = rb_hash_aref(rbOptions, symbol_italic))) {
    italic = RTEST(val);
  }
  if (preTtcIndex != -1) {
    ttcIndex = preTtcIndex;
  } else if (!NIL_P(val = rb_hash_aref(rbOptions, symbol_ttc_index))) {
    ttcIndex = NUM2INT(val);
  }
  volatile VALUE rbHashKey = rb_str_dup(rbRealFilePath);  
  char temp[256];
  // TODO: change the delimiter or the way to name a hash key
  rb_str_cat2(rbHashKey, ";size=");
  snprintf(temp, sizeof(temp), "%d", size);
  rb_str_cat2(rbHashKey, temp);
  if (bold) {
    rb_str_cat2(rbHashKey, ";bold=true");
  } else {
    rb_str_cat2(rbHashKey, ";bold=false");
  }
  if (italic) {
    rb_str_cat2(rbHashKey, ";italic=true");
  } else {
    rb_str_cat2(rbHashKey, ";italic=false");
  }
  rb_str_cat2(rbHashKey, ";ttc_index=");
  snprintf(temp, sizeof(temp), "%d", ttcIndex);
  rb_str_cat2(rbHashKey, temp);

  if (!NIL_P(val = rb_hash_aref(rbFontCache, rbHashKey))) {
    return val;
  } else {
    VALUE args[] = {
      rbRealFilePath,
      rbSize,
      bold ? Qtrue : Qfalse,
      italic ? Qtrue : Qfalse,
      INT2NUM(ttcIndex),
    };
    volatile VALUE rbNewFont =
      rb_class_new_instance(sizeof(args) / sizeof(VALUE), args, self);
    rb_hash_aset(rbFontCache, rbHashKey, rbNewFont);
    return rbNewFont;
  }
}

static VALUE
Font_alloc(VALUE klass)
{
  Font* font = ALLOC(Font);
  font->sdlFont = NULL;
  return Data_Wrap_Struct(klass, 0, Font_free, font);
}

static VALUE
Font_initialize(VALUE self, VALUE rbRealFilePath, VALUE rbSize,
                VALUE rbBold, VALUE rbItalic, VALUE rbTtcIndex)
{
  const char* path   = StringValueCStr(rbRealFilePath);
  const int size     = NUM2INT(rbSize);
  const bool bold    = RTEST(rbBold);
  const bool italic  = RTEST(rbItalic);
  const int ttcIndex = NUM2INT(rbTtcIndex);

  Font* font;
  Data_Get_Struct(self, Font, font);
  font->size = size;
  font->sdlFont = TTF_OpenFontIndex(path, size, ttcIndex);
  if (!font->sdlFont) {
    rb_raise(strb_GetStarRubyErrorClass(), "%s (%s)", TTF_GetError(), path);
  }
  const int style = TTF_STYLE_NORMAL |
    (bold ? TTF_STYLE_BOLD : 0) | (italic ? TTF_STYLE_ITALIC : 0);
  TTF_SetFontStyle(font->sdlFont, style);

  return Qnil;
}

static VALUE
Font_bold(VALUE self)
{
  const Font* font;
  Data_Get_Struct(self, Font, font);
  return (TTF_GetFontStyle(font->sdlFont) & TTF_STYLE_BOLD) ? Qtrue : Qfalse;
}

static VALUE
Font_italic(VALUE self)
{
  const Font* font;
  Data_Get_Struct(self, Font, font);
  return (TTF_GetFontStyle(font->sdlFont) & TTF_STYLE_ITALIC) ? Qtrue : Qfalse;
}

static VALUE
Font_get_size(VALUE self, VALUE rbText)
{
  const Font* font;
  Data_Get_Struct(self, Font, font);
  const char* text = StringValueCStr(rbText);
  int width, height;
  if (TTF_SizeUTF8(font->sdlFont, text, &width, &height)) {
    rb_raise_sdl_ttf_error();
  }
  volatile VALUE rbSize = rb_assoc_new(INT2NUM(width), INT2NUM(height));
  OBJ_FREEZE(rbSize);
  return rbSize;
}

static VALUE
Font_name(VALUE self)
{
  const Font* font;
  Data_Get_Struct(self, Font, font);
  return rb_str_new2(TTF_FontFaceFamilyName(font->sdlFont));
}

static VALUE
Font_size(VALUE self)
{
  const Font* font;
  Data_Get_Struct(self, Font, font);
  return INT2NUM(font->size);
}

#define ADD_INFO(currentInfo, _rbFontNameSymbol, \
                 _rbFileNameSymbol, _ttcIndex)   \
  do {                                           \
    FontFileInfo* info = ALLOC(FontFileInfo);    \
    info->rbFontNameSymbol = _rbFontNameSymbol;  \
    info->rbFileNameSymbol = _rbFileNameSymbol;  \
    info->ttcIndex         = _ttcIndex;          \
    info->next             = NULL;               \
    currentInfo->next = info;                    \
    currentInfo = info;                          \
  } while (false)

void
strb_InitializeSdlFont(void)
{
  if (TTF_Init()) {
    rb_raise_sdl_ttf_error();
  }
  fontFileInfos = ALLOC(FontFileInfo);
  fontFileInfos->rbFontNameSymbol = Qundef;
  fontFileInfos->rbFileNameSymbol = Qundef;
  fontFileInfos->ttcIndex         = -1;
  fontFileInfos->next             = NULL;
  FontFileInfo* currentInfo = fontFileInfos;
  (void)currentInfo;

#ifdef WIN32
  HKEY hKey;
  TCHAR* regPath =
    _T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts");
  if (SUCCEEDED(RegOpenKeyEx(HKEY_LOCAL_MACHINE, regPath, 0,
                             KEY_READ, &hKey))) {
    DWORD fontNameBuffMaxLength;
    DWORD fileNameBuffMaxByteLength;
    RegQueryInfoKey(hKey, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                    &fontNameBuffMaxLength, &fileNameBuffMaxByteLength,
                    NULL, NULL);
    TCHAR fontNameBuff[fontNameBuffMaxLength + 1];
    BYTE fileNameByteBuff[fileNameBuffMaxByteLength];
    for (DWORD dwIndex = 0; ;dwIndex++) {
      ZeroMemory(fontNameBuff, sizeof(fontNameBuff));
      ZeroMemory(fileNameByteBuff, sizeof(fileNameByteBuff));
      DWORD fontNameBuffLength = sizeof(fontNameBuff) / sizeof(TCHAR);
      DWORD fileNameBuffByteLength = fileNameBuffMaxByteLength;
      LONG result = RegEnumValue(hKey, dwIndex,
                                 fontNameBuff, &fontNameBuffLength,
                                 NULL, NULL,
                                 fileNameByteBuff, &fileNameBuffByteLength);
      TCHAR* fileNameBuff = (TCHAR*)fileNameByteBuff;
      DWORD fileNameBuffLength = _tcslen(fileNameBuff);
      if (result == ERROR_SUCCESS) {
        const TCHAR* ext = &(fileNameBuff[fileNameBuffLength - 3]);
        if (tolower(ext[0]) == _T('t') &&
            tolower(ext[1]) == _T('t') &&
            (tolower(ext[2]) == _T('f') ||
             tolower(ext[2]) == _T('c'))) {
          TCHAR* fontName = fontNameBuff;
          const TCHAR* fileName = fileNameBuff;
          // A TTF font name must end with ' (TrueType)'.
          fontName[fontNameBuffLength - 11] = _T('\0');
          for (int i = fileNameBuffLength - 1; 0 <= i; i--) {
            if (fileName[i] == _T('\\')) {
              fileName += i + 1;
              break;
            }
          }
          int length =
            WideCharToMultiByte(CP_UTF8, 0,
                                fontName, -1,
                                NULL, 0,
                                NULL, NULL);
          char fontNameUTF8[length];
          WideCharToMultiByte(CP_UTF8, 0,
                              fontName, -1,
                              fontNameUTF8, length,
                              NULL, NULL);
          volatile VALUE rbFontName = rb_str_new2(fontNameUTF8);
          length =
            WideCharToMultiByte(CP_ACP, 0,
                                fileName, -1,
                                NULL, 0,
                                NULL, NULL);
          char fileNameANSI[length];
          WideCharToMultiByte(CP_ACP, 0,
                              fileName, -1,
                              fileNameANSI, length,
                              NULL, NULL);
          volatile VALUE rbFileName = rb_str_new2(fileNameANSI);
          if (strchr(StringValueCStr(rbFontName), '&')) {
            volatile VALUE rbArr = rb_str_split(rbFontName, "&");
            const int arrLength = RARRAY_LEN(rbArr);
            int ttcIndex = 0;
            for (int i = 0; i < arrLength; i++) {
              volatile VALUE rbFontName = rb_ary_entry(rbArr, i);
              rb_funcall(rbFontName, rb_intern("strip!"), 0);
              if (0 < RSTRING_LEN(rbFontName)) {
                volatile VALUE rbFontNameSymbol = rb_str_intern(rbFontName);
                volatile VALUE rbFileNameSymbol = rb_str_intern(rbFileName);
                ADD_INFO(currentInfo, rbFontNameSymbol, rbFileNameSymbol,
                         ttcIndex);
                ttcIndex++;
              }
            }
          } else {
            volatile VALUE rbFontNameSymbol = rb_str_intern(rbFontName);
            volatile VALUE rbFileNameSymbol = rb_str_intern(rbFileName);
            ADD_INFO(currentInfo, rbFontNameSymbol, rbFileNameSymbol, -1);
          }
        }
      } else {
        break;
      }
    }
    RegCloseKey(hKey);
  } else {
    rb_raise(strb_GetStarRubyErrorClass(),
             "Win32API error: %d", (int)GetLastError());
  }
  TCHAR szWindowsFontDirPath[MAX_PATH + 1];
  if (FAILED(SHGetFolderPath(NULL, CSIDL_FONTS, NULL,
                             SHGFP_TYPE_CURRENT,
                             szWindowsFontDirPath))) {
    rb_raise(strb_GetStarRubyErrorClass(),
             "Win32API error: %d", (int)GetLastError());
  }
  int length =
    WideCharToMultiByte(CP_UTF8, 0,
                        szWindowsFontDirPath, -1,
                        NULL, 0,
                        NULL, NULL);
  char szWindowsFontDirPathUTF8[length];
  WideCharToMultiByte(CP_UTF8, 0,
                      szWindowsFontDirPath, -1,
                      szWindowsFontDirPathUTF8, length,
                      NULL, NULL);
  volatile VALUE rbWindowsFontDirPath = rb_str_new2(szWindowsFontDirPathUTF8);
  rbWindowsFontDirPathSymbol = rb_str_intern(rbWindowsFontDirPath);
#endif
}

VALUE
strb_InitializeFont(VALUE rb_mStarRuby)
{
  VALUE rb_cFont = rb_define_class_under(rb_mStarRuby, "Font", rb_cObject);
  rb_define_singleton_method(rb_cFont, "exist?", Font_s_exist, 1);
  rb_define_singleton_method(rb_cFont, "new",    Font_s_new,   -1);
  rb_define_alloc_func(rb_cFont, Font_alloc);
  rb_define_private_method(rb_cFont, "initialize", Font_initialize, 5);
  rb_define_method(rb_cFont, "bold?",     Font_bold,     0);
  rb_define_method(rb_cFont, "get_size",  Font_get_size, 1);
  rb_define_method(rb_cFont, "italic?",   Font_italic,   0);
  rb_define_method(rb_cFont, "name",      Font_name,     0);
  rb_define_method(rb_cFont, "size",      Font_size,     0);

  symbol_bold      = ID2SYM(rb_intern("bold"));
  symbol_italic    = ID2SYM(rb_intern("italic"));
  symbol_ttc_index = ID2SYM(rb_intern("ttc_index"));

  rbFontCache = rb_hash_new();
  rb_gc_register_address(&rbFontCache);

  return rb_cFont;
}
