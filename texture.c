#include "starruby.h"

#define rb_raise_sdl_image_error() rb_raise(rb_eStarRubyError, IMG_GetError())
#define DIV255(x) ((x + 255) >> 8)
#define ALPHA(src, dst, a) (DIV255((dst << 8) - dst + (src - dst) * a))

static VALUE symbol_src_x;
static VALUE symbol_src_y;
static VALUE symbol_src_width;
static VALUE symbol_src_height;
static VALUE symbol_alpha;
static VALUE symbol_blend_type;
static VALUE symbol_tone_red;
static VALUE symbol_tone_green;
static VALUE symbol_tone_blue;
static VALUE symbol_tone_saturation;
static VALUE symbol_add;
static VALUE symbol_sub;

typedef enum {
  ALPHA,
  ADD,
  SUB,
} BlendType;

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

  Texture* texture;
  Data_Get_Struct(rbTexture, Texture, texture);

  SDL_LockSurface(surface);
  MEMCPY(texture->pixels, surface->pixels, Pixel,
         texture->width * texture->height);
  SDL_UnlockSurface(surface);
  
  SDL_FreeSurface(surface);
  SDL_FreeSurface(imageSurface);
  
  return rbTexture;
}

static void Texture_free(Texture* texture)
{
  free(texture->pixels);
  free(texture);
}

static VALUE Texture_alloc(VALUE klass)
{
  Texture* texture = ALLOC(Texture);
  return Data_Wrap_Struct(klass, 0, Texture_free, texture);
}

static VALUE Texture_initialize(VALUE self, VALUE rbWidth, VALUE rbHeight)
{
  Texture* texture;
  Data_Get_Struct(self, Texture, texture);
  
  texture->width  = NUM2INT(rbWidth);
  texture->height = NUM2INT(rbHeight);
  texture->pixels = ALLOC_N(Pixel, texture->width * texture->height);
  MEMZERO(texture->pixels, Pixel, texture->width * texture->height);
  return Qnil;
}

static VALUE Texture_initialize_copy(VALUE self, VALUE rbTexture)
{
  Texture* texture;
  Data_Get_Struct(self, Texture, texture);

  Texture* origTexture;
  Data_Get_Struct(rbTexture, Texture, origTexture);

  texture->width  = origTexture->width;
  texture->height = origTexture->height;
  int length = texture->width * texture->height;
  texture->pixels = ALLOC_N(Pixel, length);
  MEMCPY(texture->pixels, origTexture->pixels, Pixel, length);
  
  return Qnil;
}

static VALUE Texture_change_hue(VALUE self, VALUE rbAngle)
{
  rb_check_frozen(self);
  
  Texture* texture;
  Data_Get_Struct(self, Texture, texture);
  if (!texture->pixels) {
    rb_raise(rb_eTypeError, "can't modify disposed texture");
    return Qnil;
  }
  
  double angle = NUM2DBL(rbAngle);
  if (angle == 0)
    return Qnil;
  int length = texture->width * texture->height;
  Pixel* pixel = texture->pixels;
  for (int i = 0; i < length; i++, pixel++) {
    uint8_t r = pixel->color.red;
    uint8_t g = pixel->color.green;
    uint8_t b = pixel->color.blue;
    uint8_t max = MAX(MAX(r, g), b);
    uint8_t min = MIN(MIN(r, g), b);
    if (max == 0)
      continue;
    double delta255 = max - min;
    double v = max / 255.0;
    double s = delta255 / max;
    double h;
    if (max == r)
      h =     (g - b) / delta255;
    else if (max == g)
      h = 2 + (b - r) / delta255;
    else
      h = 4 + (r - g) / delta255;
    if (h < 0.0)
      h += 6.0;
    h += angle * 6.0 / (2 * PI);
    if (6.0 <= h)
      h -= 6.0;
    int ii = (int)floor(h);
    double f = h - ii;
    uint8_t v255 = max;
    uint8_t aa255 = (uint8_t)(v * (1 - s) * 255);
    uint8_t bb255 = (uint8_t)(v * (1 - s * f) * 255);
    uint8_t cc255 = (uint8_t)(v * (1 - s * (1 - f)) * 255);
    switch (ii) {
    case 0:
      r = v255;  g = cc255; b = aa255;
      break;
    case 1:
      r = bb255; g = v255;  b = aa255;
      break;
    case 2:
      r = aa255; g = v255;  b = cc255;
      break;
    case 3:
      r = aa255; g = bb255; b = v255;
      break;
    case 4:
      r = cc255; g = aa255; b = v255;
      break;
    case 5:
      r = v255;  g = aa255; b = bb255;
      break;
    }
    pixel->color.red   = r;
    pixel->color.green = g;
    pixel->color.blue  = b;
  }

  return Qnil;
}

static VALUE Texture_clear(VALUE self)
{
  rb_check_frozen(self);
  
  Texture* texture;
  Data_Get_Struct(self, Texture, texture);
  if (!texture->pixels) {
    rb_raise(rb_eTypeError, "can't modify disposed texture");
    return Qnil;
  }
  
  MEMZERO(texture->pixels, Color, texture->width * texture->height);
  return Qnil;
}

static VALUE Texture_dispose(VALUE self)
{
  Texture* texture;
  Data_Get_Struct(self, Texture, texture);
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
  Texture* texture;
  Data_Get_Struct(self, Texture, texture);
  return !texture->pixels ? Qtrue : Qfalse;
}

static VALUE Texture_fill(VALUE self, VALUE rbColor)
{
  rb_check_frozen(self);
  
  Texture* texture;
  Data_Get_Struct(self, Texture, texture);
  if (!texture->pixels) {
    rb_raise(rb_eTypeError, "can't modify disposed texture");
    return Qnil;
  }
  
  Color* color;
  Data_Get_Struct(rbColor, Color, color);

  int length = texture->width * texture->height;
  Pixel* pixels = texture->pixels;
  for (int i = 0; i < length; i++, pixels++)
    pixels->color = *color;
  
  return Qnil;
}

static VALUE Texture_fill_rect(VALUE self,
                               VALUE rbX, VALUE rbY,
                               VALUE rbWidth, VALUE rbHeight, VALUE rbColor)
{
  rb_check_frozen(self);
  
  Texture* texture;
  Data_Get_Struct(self, Texture, texture);
  if (!texture->pixels) {
    rb_raise(rb_eTypeError, "can't modify disposed texture");
    return Qnil;
  }

  int rectX = NUM2INT(rbX);
  int rectY = NUM2INT(rbY);
  int rectWidth = NUM2INT(rbWidth);
  int rectHeight= NUM2INT(rbHeight);
  Color* color;
  Data_Get_Struct(rbColor, Color, color);

  int width = texture->width;
  Pixel* pixels = texture->pixels;
  
  for (int j = rectY; j < rectY + rectHeight; j++)
    for (int i = rectX; i < rectX + rectWidth; i++)
      pixels[i + j * width].color = *color;
  
  return Qnil;
}

static VALUE Texture_get_pixel(VALUE self, VALUE rbX, VALUE rbY)
{
  int x = NUM2INT(rbX);
  int y = NUM2INT(rbY);
  
  Texture* texture;
  Data_Get_Struct(self, Texture, texture);
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
  
  Color color = texture->pixels[x + y * texture->width].color;
  return rb_funcall(rb_cColor, rb_intern("new"), 4,
                    INT2NUM(color.red),
                    INT2NUM(color.green),
                    INT2NUM(color.blue),
                    INT2NUM(color.alpha));
}

static VALUE Texture_height(VALUE self)
{
  Texture* texture;
  Data_Get_Struct(self, Texture, texture);
  return INT2NUM(texture->height);
}

static VALUE Texture_render_texture(int argc, VALUE* argv, VALUE self)
{
  rb_check_frozen(self);
  
  Texture* dstTexture;
  Data_Get_Struct(self, Texture, dstTexture);
  if (!dstTexture->pixels) {
    rb_raise(rb_eTypeError, "can't modify disposed texture");
    return Qnil;
  }

  VALUE rbTexture, rbX, rbY, rbOptions;
  rb_scan_args(argc, argv, "31", &rbTexture, &rbX, &rbY, &rbOptions);
  if (rbOptions == Qnil)
    rbOptions = rb_hash_new();

  Texture* srcTexture;
  Data_Get_Struct(rbTexture, Texture, srcTexture);
  if (!srcTexture->pixels) {
    rb_raise(rb_eTypeError, "can't use disposed texture");
    return Qnil;
  }

  int srcTextureWidth = srcTexture->width;
  int srcTextureHeight = srcTexture->height;
  int dstTextureWidth = dstTexture->width;
  int dstTextureHeight = dstTexture->height;

  VALUE rbSrcX           = rb_hash_aref(rbOptions, symbol_src_x);
  VALUE rbSrcY           = rb_hash_aref(rbOptions, symbol_src_y);
  VALUE rbSrcWidth       = rb_hash_aref(rbOptions, symbol_src_width);
  VALUE rbSrcHeight      = rb_hash_aref(rbOptions, symbol_src_height);
  VALUE rbAlpha          = rb_hash_aref(rbOptions, symbol_alpha);
  VALUE rbBlendType      = rb_hash_aref(rbOptions, symbol_blend_type);
  VALUE rbToneRed        = rb_hash_aref(rbOptions, symbol_tone_red);
  VALUE rbToneGreen      = rb_hash_aref(rbOptions, symbol_tone_green);
  VALUE rbToneBlue       = rb_hash_aref(rbOptions, symbol_tone_blue);
  VALUE rbToneSaturation = rb_hash_aref(rbOptions, symbol_tone_saturation);
  
  int srcX = (rbSrcX != Qnil) ? NUM2INT(rbSrcX) : 0;
  int srcY = (rbSrcY != Qnil) ? NUM2INT(rbSrcY) : 0;
  int srcWidth =
    (rbSrcWidth != Qnil) ? NUM2INT(rbSrcWidth) : (srcTextureWidth - srcX);
  int srcHeight =
    (rbSrcHeight != Qnil) ? NUM2INT(rbSrcHeight) : (srcTextureHeight - srcY);
  int alpha = (rbAlpha != Qnil) ? NORMALIZE(NUM2INT(rbAlpha), 0, 255) : 255;
  BlendType blendType = ALPHA;
  if (rbBlendType == Qnil || rbBlendType == symbol_alpha) {
    blendType = ALPHA;
  } else if (rbBlendType == symbol_add) {
    blendType = ADD;
  } else if (rbBlendType == symbol_sub) {
    blendType = SUB;
  }

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

  Pixel* dst = &(dstTexture->pixels[dstX + dstY * dstTextureWidth]);
  Pixel* src = &(srcTexture->pixels[srcX + srcY * srcTextureWidth]);
  uint8_t srcAlpha;
  for (int j = 0; j < srcHeight; j++) {
    for (int i = 0; i < srcWidth; i++) {
      srcAlpha = DIV255(src->color.alpha * alpha);
      dst->color.alpha = MAX(dst->color.alpha, srcAlpha);
      switch (blendType) {
      case ALPHA:
        dst->color.red   = ALPHA(src->color.red,   dst->color.red,   srcAlpha);
        dst->color.green = ALPHA(src->color.green, dst->color.green, srcAlpha);
        dst->color.blue  = ALPHA(src->color.blue,  dst->color.blue,  srcAlpha);
        break;
      case ADD:
        dst->color.red =
          MIN(255, dst->color.red + DIV255(src->color.red * srcAlpha));
        dst->color.green =
          MIN(255, dst->color.green + DIV255(src->color.green * srcAlpha));
        dst->color.blue =
          MIN(255, dst->color.blue + DIV255(src->color.blue * srcAlpha));
        break;
      case SUB:
        dst->color.red =
          MAX(0, (int)dst->color.red - DIV255(src->color.red * srcAlpha));
        dst->color.green =
          MAX(0, (int)dst->color.green - DIV255(src->color.green * srcAlpha));
        dst->color.blue =
          MAX(0, (int)dst->color.blue - DIV255(src->color.blue * srcAlpha));
        break;
      }
      dst++;
      src++;
    }
    dst += -srcWidth + dstTextureWidth;
    src += -srcWidth + srcTextureWidth;
  }

  return Qnil;
}

static VALUE Texture_set_pixel(VALUE self, VALUE rbX, VALUE rbY, VALUE rbColor)
{
  rb_check_frozen(self);
  
  Texture* texture;
  Data_Get_Struct(self, Texture, texture);

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

  Color* color;
  Data_Get_Struct(rbColor, Color, color);
  
  texture->pixels[x + y * texture->width].color = *color;
  return rbColor;
}

static VALUE Texture_size(VALUE self)
{
  Texture* texture;
  Data_Get_Struct(self, Texture, texture);
  VALUE rbSize = rb_assoc_new(INT2NUM(texture->width),
                              INT2NUM(texture->height));
  rb_obj_freeze(rbSize);
  return rbSize;
}

static VALUE Texture_width(VALUE self)
{
  Texture* texture;
  Data_Get_Struct(self, Texture, texture);
  return INT2NUM(texture->width);
}

void InitializeTexture(void)
{
  rb_cTexture = rb_define_class_under(rb_mStarRuby, "Texture", rb_cObject);
  rb_define_singleton_method(rb_cTexture, "load", Texture_load, 1);
  rb_define_alloc_func(rb_cTexture, Texture_alloc);
  rb_define_private_method(rb_cTexture, "initialize", Texture_initialize, 2);
  rb_define_private_method(rb_cTexture, "initialize_copy",
                           Texture_initialize_copy, 1);
  rb_define_method(rb_cTexture, "change_hue",     Texture_change_hue,     1);
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

  symbol_src_x           = ID2SYM(rb_intern("src_x"));
  symbol_src_y           = ID2SYM(rb_intern("src_y"));
  symbol_src_width       = ID2SYM(rb_intern("src_width"));
  symbol_src_height      = ID2SYM(rb_intern("src_height"));
  symbol_alpha           = ID2SYM(rb_intern("alpha"));
  symbol_blend_type      = ID2SYM(rb_intern("blend_type"));
  symbol_tone_red        = ID2SYM(rb_intern("tone_red"));
  symbol_tone_green      = ID2SYM(rb_intern("tone_green"));
  symbol_tone_blue       = ID2SYM(rb_intern("tone_blue"));
  symbol_tone_saturation = ID2SYM(rb_intern("tone_saturation"));
  symbol_add             = ID2SYM(rb_intern("add"));
  symbol_sub             = ID2SYM(rb_intern("sub"));
}
