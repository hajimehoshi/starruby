#include "starruby.h"

#define rb_raise_sdl_image_error() rb_raise(rb_eStarRubyError, IMG_GetError())

static void Texture_free(struct Texture* texture)
{
  free(texture->pixels);
  free(texture);
}

static VALUE Texture_alloc(VALUE klass)
{
  struct Texture* texture = ALLOC(struct Texture);
  return Data_Wrap_Struct(klass, 0, Texture_free, texture);
}

static VALUE Texture_initialize(VALUE self, VALUE rbWidth, VALUE rbHeight)
{
  struct Texture* texture;
  Data_Get_Struct(self, struct Texture, texture);
  
  texture->width  = NUM2INT(rbWidth);
  texture->height = NUM2INT(rbHeight);
  texture->pixels = ALLOC_N(union Pixel, texture->width * texture->height);
  MEMZERO(texture->pixels, union Pixel, texture->width * texture->height);
  return Qnil;
}

static VALUE Texture_initialize_copy(VALUE self, VALUE rbTexture)
{
  struct Texture* texture;
  Data_Get_Struct(self, struct Texture, texture);

  struct Texture* origTexture;
  Data_Get_Struct(rbTexture, struct Texture, origTexture);

  texture->width  = origTexture->width;
  texture->height = origTexture->height;
  int length = texture->width * texture->height;
  texture->pixels = ALLOC_N(union Pixel, length);
  MEMCPY(texture->pixels, origTexture->pixels, union Pixel, length);
  
  return Qnil;
}

static VALUE Texture_load(VALUE self, VALUE rbPath)
{
  char* path = StringValuePtr(rbPath);
  
  if (rb_funcall(rb_mFileTest, rb_intern("file?"), 1, rbPath) == Qfalse) {
    VALUE rbPathes = rb_funcall(rb_cDir, rb_intern("[]"), 1,
                                rb_str_cat2(rb_str_dup(rbPath), ".*"));
    if (1 <= RARRAY(rbPathes)->len) {
      rb_ary_sort_bang(rbPathes);
      VALUE rbNewPath = rb_ary_shift(rbPathes);
      path = StringValuePtr(rbNewPath);
    } else {
      VALUE rbENOENT = rb_const_get(rb_mErrno, rb_intern("ENOENT"));
      rb_raise(rbENOENT, path);
      return Qnil;
    }
  }
  
  SDL_Surface* imageSurface = IMG_Load(path);
  if (!imageSurface) {
    rb_raise_sdl_image_error();
    return Qnil;
  }

  SDL_Surface* surface = SDL_DisplayFormatAlpha(imageSurface);
  if (!surface) {
    rb_raise_sdl_error();
    return Qnil;
  }
  
  VALUE rbTexture = rb_funcall(self, rb_intern("new"), 2,
                               INT2NUM(surface->w), INT2NUM(surface->h));

  struct Texture* texture;
  Data_Get_Struct(rbTexture, struct Texture, texture);

  SDL_LockSurface(surface);
  MEMCPY(texture->pixels, surface->pixels, union Pixel,
         texture->width * texture->height);
  SDL_UnlockSurface(surface);
  
  SDL_FreeSurface(surface);
  SDL_FreeSurface(imageSurface);
  
  return rbTexture;
}

static VALUE Texture_clear(VALUE self)
{
  rb_check_frozen(self);
  
  struct Texture* texture;
  Data_Get_Struct(self, struct Texture, texture);
  if (!texture->pixels) {
    rb_raise(rb_eTypeError, "can't modify disposed texture");
    return Qnil;
  }
  
  MEMZERO(texture->pixels, struct Color, texture->width * texture->height);
  return Qnil;
}

static VALUE Texture_dispose(VALUE self)
{
  struct Texture* texture;
  Data_Get_Struct(self, struct Texture, texture);
  if (!texture->pixels) {
    rb_raise(rb_eStarRubyError, "already disposed");
    return Qnil;
  }
  free(texture->pixels);
  texture->pixels = NULL;
  return Qnil;
}

static VALUE Texture_disposed(VALUE self)
{
  struct Texture* texture;
  Data_Get_Struct(self, struct Texture, texture);
  return !texture->pixels ? Qtrue : Qfalse;
}

static VALUE Texture_fill(VALUE self, VALUE rbColor)
{
  rb_check_frozen(self);
  
  struct Texture* texture;
  Data_Get_Struct(self, struct Texture, texture);
  if (!texture->pixels) {
    rb_raise(rb_eTypeError, "can't modify disposed texture");
    return Qnil;
  }
  
  struct Color* color;
  Data_Get_Struct(rbColor, struct Color, color);

  int length = texture->width * texture->height;
  union Pixel* pixels = texture->pixels;
  for (int i = 0; i < length; i++, pixels++)
    pixels->color = *color;
  
  return Qnil;
}

static VALUE Texture_fill_rect(VALUE self,
                               VALUE rbX, VALUE rbY,
                               VALUE rbWidth, VALUE rbHeight, VALUE rbColor)
{
  rb_check_frozen(self);
  
  struct Texture* texture;
  Data_Get_Struct(self, struct Texture, texture);
  if (!texture->pixels) {
    rb_raise(rb_eTypeError, "can't modify disposed texture");
    return Qnil;
  }

  int rectX = NUM2INT(rbX);
  int rectY = NUM2INT(rbY);
  int rectWidth = NUM2INT(rbWidth);
  int rectHeight= NUM2INT(rbHeight);
  struct Color* color;
  Data_Get_Struct(rbColor, struct Color, color);

  int width = texture->width;
  union Pixel* pixels = texture->pixels;
  
  for (int j = rectY; j < rectY + rectHeight; j++)
    for (int i = rectX; i < rectX + rectWidth; i++)
      pixels[i + j * width].color = *color;
  
  return Qnil;
}

static VALUE Texture_get_pixel(VALUE self, VALUE rbX, VALUE rbY)
{
  int x = NUM2INT(rbX);
  int y = NUM2INT(rbY);
  
  struct Texture* texture;
  Data_Get_Struct(self, struct Texture, texture);
  if (!texture->pixels) {
    rb_raise(rb_eTypeError, "can't modify disposed texture");
    return Qnil;
  }
  
  if (x < 0 || texture->width <= x || y < 0 || texture->height <= y) {
    char errorMessage[256];
    snprintf(errorMessage, sizeof(errorMessage),
             "index out of range: (%d, %d)", x, y);
    rb_raise(rb_eIndexError, errorMessage);
    return Qnil;
  }
  
  struct Color color = texture->pixels[x + y * texture->width].color;
  return rb_funcall(rb_cColor, rb_intern("new"), 4,
                    INT2NUM(color.red),
                    INT2NUM(color.green),
                    INT2NUM(color.blue),
                    INT2NUM(color.alpha));
}

static VALUE Texture_height(VALUE self)
{
  struct Texture* texture;
  Data_Get_Struct(self, struct Texture, texture);
  return INT2NUM(texture->height);
}

static VALUE Texture_render_texture(int argc, VALUE* argv, VALUE self)
{
  rb_check_frozen(self);
  
  struct Texture* dstTexture;
  Data_Get_Struct(self, struct Texture, dstTexture);
  if (!dstTexture->pixels) {
    rb_raise(rb_eTypeError, "can't modify disposed texture");
    return Qnil;
  }

  VALUE rbTexture, rbX, rbY, rbOptions;
  rb_scan_args(argc, argv, "31", &rbTexture, &rbX, &rbY, &rbOptions);
  if (rbOptions == Qnil)
    rbOptions = rb_hash_new();

  struct Texture* srcTexture;
  Data_Get_Struct(rbTexture, struct Texture, srcTexture);
  if (!srcTexture->pixels) {
    rb_raise(rb_eTypeError, "can't use disposed texture");
    return Qnil;
  }

  int srcTextureWidth = srcTexture->width;
  int srcTextureHeight = srcTexture->height;
  int dstTextureWidth = dstTexture->width;
  int dstTextureHeight = dstTexture->height;

  VALUE rbSrcX = rb_hash_aref(rbOptions, rb_intern("src_x"));
  VALUE rbSrcY = rb_hash_aref(rbOptions, rb_intern("src_y"));
  VALUE rbSrcWidth = rb_hash_aref(rbOptions, rb_intern("src_width"));
  VALUE rbSrcHeight = rb_hash_aref(rbOptions, rb_intern("src_height"));
  
  int srcX = (rbSrcX != Qnil) ? NUM2INT(rbSrcX) : 0;
  int srcY = (rbSrcY != Qnil) ? NUM2INT(rbSrcY) : 0;
  int srcWidth =
    (rbSrcWidth != Qnil) ? NUM2INT(rbSrcWidth) : srcTextureWidth;
  int srcHeight =
    (rbSrcHeight != Qnil) ? NUM2INT(rbSrcHeight) : srcTextureHeight;

  int dstX = NUM2INT(rbX);
  int dstY = NUM2INT(rbY);
  if (dstX < 0) {
    srcX += -dstX;
    srcWidth -= -dstX;
    dstX = 0;
  }
  if (dstY < 0) {
    srcY += -dstY;
    srcHeight -= -dstY;
    dstY = 0;
  }

  if (srcX < 0 || srcY < 0 || srcWidth < 0 || srcHeight < 0 ||
      srcTextureWidth < srcX + srcWidth || srcTextureHeight < srcY + srcHeight)
    return Qnil; // RangeError?
  if (dstTextureWidth <= dstX || dstTextureHeight <= dstY)
    return Qnil;

  srcWidth = MIN(srcWidth, dstTextureWidth - dstX);
  srcHeight = MIN(srcHeight, dstTextureHeight - dstY);

  for (int j = 0; j < srcHeight; j++) {
    MEMCPY(dstTexture->pixels + dstX + (j + dstY) * dstTextureWidth,
           srcTexture->pixels + srcX + (j + srcY) * srcTextureWidth,
           union Pixel, srcWidth);
  }

  return Qnil;
}

static VALUE Texture_set_pixel(VALUE self, VALUE rbX, VALUE rbY, VALUE rbColor)
{
  rb_check_frozen(self);
  
  struct Texture* texture;
  Data_Get_Struct(self, struct Texture, texture);

  if (!texture->pixels) {
    rb_raise(rb_eTypeError, "can't modify disposed texture");
    return Qnil;
  }

  int x = NUM2INT(rbX);
  int y = NUM2INT(rbY);
  
  if (x < 0 || texture->width <= x || y < 0 || texture->height <= y) {
    char errorMessage[256];
    snprintf(errorMessage, sizeof(errorMessage),
             "index out of range: (%d, %d)", x, y);
    rb_raise(rb_eIndexError, errorMessage);
    return Qnil;
  }

  struct Color* color;
  Data_Get_Struct(rbColor, struct Color, color);
  
  texture->pixels[x + y * texture->width].color = *color;
  return rbColor;
}

static VALUE Texture_size(VALUE self)
{
  struct Texture* texture;
  Data_Get_Struct(self, struct Texture, texture);
  VALUE rbSize = rb_assoc_new(INT2NUM(texture->width),
                              INT2NUM(texture->height));
  rb_obj_freeze(rbSize);
  return rbSize;
}

static VALUE Texture_width(VALUE self)
{
  struct Texture* texture;
  Data_Get_Struct(self, struct Texture, texture);
  return INT2NUM(texture->width);
}

void InitializeTexture(void)
{
  rb_cTexture = rb_define_class_under(rb_mStarRuby, "Texture", rb_cObject);
  rb_define_alloc_func(rb_cTexture, Texture_alloc);
  rb_define_private_method(rb_cTexture, "initialize", Texture_initialize, 2);
  rb_define_private_method(rb_cTexture, "initialize_copy",
                           Texture_initialize_copy, 1);
  rb_define_singleton_method(rb_cTexture, "load", Texture_load, 1);
  rb_define_method(rb_cTexture, "clear",          Texture_clear,          0);
  rb_define_method(rb_cTexture, "dispose",        Texture_dispose,        0);
  rb_define_method(rb_cTexture, "disposed?",      Texture_disposed,       0);
  rb_define_method(rb_cTexture, "fill",           Texture_fill,           1);
  rb_define_method(rb_cTexture, "fill_rect",      Texture_fill_rect,      5);
  rb_define_method(rb_cTexture, "get_pixel",      Texture_get_pixel,      2);
  rb_define_method(rb_cTexture, "height",         Texture_height,         0);
  rb_define_method(rb_cTexture, "render_texture", Texture_render_texture, -1);
  rb_define_method(rb_cTexture, "set_pixel",      Texture_set_pixel,      3);
  rb_define_method(rb_cTexture, "size",           Texture_size,           0);
  rb_define_method(rb_cTexture, "width",          Texture_width,          0);
}
