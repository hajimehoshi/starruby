#include "starruby.h"

#define DIV255(x) ((x + 255) >> 8)
#define ALPHA(src, dst, a) (DIV255((dst << 8) - dst + (src - dst) * a))

static VALUE symbol_add;
static VALUE symbol_alpha;
static VALUE symbol_angle;
static VALUE symbol_blend_type;
static VALUE symbol_center_x;
static VALUE symbol_center_y;
static VALUE symbol_saturation;
static VALUE symbol_scale_x;
static VALUE symbol_scale_y;
static VALUE symbol_src_height;
static VALUE symbol_src_width;
static VALUE symbol_src_x;
static VALUE symbol_src_y;
static VALUE symbol_sub;
static VALUE symbol_tone_blue;
static VALUE symbol_tone_green;
static VALUE symbol_tone_red;

typedef enum {
  ALPHA,
  ADD,
  SUB,
} BlendType;

static VALUE Texture_load(VALUE self, VALUE rbPath)
{
  char* path = StringValuePtr(rbPath);
  
  if (!RTEST(rb_funcall(rb_mFileTest, rb_intern("file?"), 1, rbPath))) {
    VALUE rbPathes = rb_funcall(rb_cDir, rb_intern("[]"), 1,
                                rb_str_cat2(rb_str_dup(rbPath), ".*"));
    if (1 <= RARRAY(rbPathes)->len) {
      rb_ary_sort_bang(rbPathes);
      VALUE rbNewPath = rb_ary_shift(rbPathes);
      path = StringValuePtr(rbNewPath);
    } else {
      rb_raise(rb_path2class("Errno::ENOENT"), "%s", path);
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
    SDL_FreeSurface(imageSurface);
    imageSurface = NULL;
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
  surface = NULL;
  SDL_FreeSurface(imageSurface);
  imageSurface = NULL;
  
  return rbTexture;
}

static void Texture_free(Texture* texture)
{
  free(texture->pixels);
  texture->pixels = NULL;
  free(texture);
}

static VALUE Texture_alloc(VALUE klass)
{
  Texture* texture = ALLOC(Texture);
  texture->pixels = NULL;
  return Data_Wrap_Struct(klass, 0, Texture_free, texture);
}

static VALUE Texture_initialize(VALUE self, VALUE rbWidth, VALUE rbHeight)
{
  Texture* texture;
  Data_Get_Struct(self, Texture, texture);

  int width  = NUM2INT(rbWidth);
  int height = NUM2INT(rbHeight);
  if (width <= 0) {
    rb_raise(rb_eArgError, "negative width");
    return Qnil;
  }
  if (height <= 0) {
    rb_raise(rb_eArgError, "negative height");
    return Qnil;
  }
  texture->width  = width;
  texture->height = height;
  
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
    rb_raise(rb_eArgError, "index out of range: (%d, %d)", x, y);
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

static VALUE Texture_render_text(VALUE self, VALUE rbText, VALUE rbX, VALUE rbY,
                                 VALUE rbFont, VALUE rbColor)
{
  rb_check_frozen(self);
  
  Texture* texture;
  Data_Get_Struct(self, Texture, texture);
  if (!texture->pixels) {
    rb_raise(rb_eTypeError, "can't modify disposed texture");
    return Qnil;
  }

  Font* font;
  Data_Get_Struct(rbFont, Font, font);
  if (!font->sdlFont) {
    rb_raise(rb_eTypeError, "can't use disposed font");
    return Qnil;
  }
  
  Color* color;
  Data_Get_Struct(rbColor, Color, color);

  char* text = StringValuePtr(rbText);
  
  int dstX = NUM2INT(rbX);
  int dstY = NUM2INT(rbY);
  if (texture->width <= dstX || texture->height <= dstY)
    return Qnil;
  
  SDL_Surface* textSurfaceRaw =
    TTF_RenderUTF8_Solid(font->sdlFont, text, (SDL_Color) {255, 255, 255, 255});
  if (!textSurfaceRaw) {
    rb_raise_sdl_ttf_error();
    return Qnil;
  }
  SDL_Surface* textSurface = SDL_DisplayFormat(textSurfaceRaw);
  if (!textSurface) {
    SDL_FreeSurface(textSurfaceRaw);
    textSurfaceRaw = NULL;
    rb_raise_sdl_error();
    return Qnil;
  }

  SDL_LockSurface(textSurface);
  
  int srcX = 0;
  int srcY = 0;
  int width = textSurface->w;
  int height = textSurface->h;
  if (dstX < 0) {
    srcX += -dstX;
    width -= -dstX;
    dstX = 0;
  }
  if (dstY < 0) {
    srcY += -dstY;
    height -= -dstY;
    dstY = 0;
  }
  if (width < 0 || height < 0 ||
      textSurface->w < srcX + width || textSurface->h < srcY + height)
    goto EXIT;

  width  = MIN(width,  texture->width  - dstX);
  height = MIN(height, texture->height - dstY);

  Pixel* src = &(((Pixel*)textSurface->pixels)[srcX + srcY * textSurface->w]);
  Pixel* dst = &(texture->pixels[dstX + dstY * texture->width]);

  for (int j = 0; j < height; j++,
       src += -width + textSurface->w, dst += -width + texture->width) {
    for (int i = 0; i < width; i++, src++, dst++) {
      if (src->value)
        dst->color = *color;
    }
  }

EXIT:
  SDL_UnlockSurface(textSurface);
  SDL_FreeSurface(textSurface);
  textSurface = NULL;
  SDL_FreeSurface(textSurfaceRaw);
  textSurfaceRaw = NULL;
  return Qnil;
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
  if (NIL_P(rbOptions))
    rbOptions = rb_hash_new();

  Texture* srcTexture;
  Data_Get_Struct(rbTexture, Texture, srcTexture);
  if (!srcTexture->pixels) {
    rb_raise(rb_eTypeError, "can't use disposed texture");
    return Qnil;
  }
  VALUE rbClonedTexture = Qnil;
  if (self == rbTexture) {
    rbClonedTexture = rb_funcall(rbTexture, rb_intern("clone"), 0);
    Data_Get_Struct(rbClonedTexture, Texture, srcTexture);
  }

  int srcTextureWidth = srcTexture->width;
  int srcTextureHeight = srcTexture->height;
  int dstTextureWidth = dstTexture->width;
  int dstTextureHeight = dstTexture->height;

  VALUE rbSrcX       = rb_hash_aref(rbOptions, symbol_src_x);
  VALUE rbSrcY       = rb_hash_aref(rbOptions, symbol_src_y);
  VALUE rbSrcWidth   = rb_hash_aref(rbOptions, symbol_src_width);
  VALUE rbSrcHeight  = rb_hash_aref(rbOptions, symbol_src_height);
  VALUE rbScaleX     = rb_hash_aref(rbOptions, symbol_scale_x);
  VALUE rbScaleY     = rb_hash_aref(rbOptions, symbol_scale_y);
  VALUE rbAngle      = rb_hash_aref(rbOptions, symbol_angle);
  VALUE rbCenterX    = rb_hash_aref(rbOptions, symbol_center_x);
  VALUE rbCenterY    = rb_hash_aref(rbOptions, symbol_center_y);
  VALUE rbAlpha      = rb_hash_aref(rbOptions, symbol_alpha);
  VALUE rbBlendType  = rb_hash_aref(rbOptions, symbol_blend_type);
  VALUE rbToneRed    = rb_hash_aref(rbOptions, symbol_tone_red);
  VALUE rbToneGreen  = rb_hash_aref(rbOptions, symbol_tone_green);
  VALUE rbToneBlue   = rb_hash_aref(rbOptions, symbol_tone_blue);
  VALUE rbSaturation = rb_hash_aref(rbOptions, symbol_saturation);
  
  int srcX = (!NIL_P(rbSrcX)) ? NUM2INT(rbSrcX) : 0;
  int srcY = (!NIL_P(rbSrcY)) ? NUM2INT(rbSrcY) : 0;
  int srcWidth =
    (!NIL_P(rbSrcWidth)) ? NUM2INT(rbSrcWidth) : (srcTextureWidth - srcX);
  int srcHeight =
    (!NIL_P(rbSrcHeight)) ? NUM2INT(rbSrcHeight) : (srcTextureHeight - srcY);
  double scaleX = !NIL_P(rbScaleX)  ? NUM2DBL(rbScaleX)  : 1;
  double scaleY = !NIL_P(rbScaleY)  ? NUM2DBL(rbScaleY)  : 1;
  double angle  = !NIL_P(rbAngle)   ? NUM2DBL(rbAngle)   : 0;
  int centerX   = !NIL_P(rbCenterX) ? NUM2INT(rbCenterX) : 0;
  int centerY   = !NIL_P(rbCenterY) ? NUM2INT(rbCenterY) : 0;
  int alpha = !NIL_P(rbAlpha) ? NORMALIZE(NUM2INT(rbAlpha), 0, 255) : 255;
  BlendType blendType = ALPHA;
  if (NIL_P(rbBlendType) || rbBlendType == symbol_alpha)
    blendType = ALPHA;
  else if (rbBlendType == symbol_add)
    blendType = ADD;
  else if (rbBlendType == symbol_sub)
    blendType = SUB;
  int toneRed =
    !NIL_P(rbToneRed)    ? NORMALIZE(NUM2INT(rbToneRed), -255, 255)   : 0;
  int toneGreen =
    !NIL_P(rbToneGreen)  ? NORMALIZE(NUM2INT(rbToneGreen), -255, 255) : 0;
  int toneBlue =
    !NIL_P(rbToneBlue)   ? NORMALIZE(NUM2INT(rbToneBlue), -255, 255)  : 0;
  uint8_t saturation =
    !NIL_P(rbSaturation) ? NORMALIZE(NUM2INT(rbSaturation), 0, 255)   : 255;
  
  AffineMatrix mat = {
    .a = 1, .c = 0, .tx = 0,
    .b = 0, .d = 1, .ty = 0,
  };
  if (scaleX != 1 || scaleY != 1 || angle != 0) {
    AffineMatrix_Concat(&mat, &(AffineMatrix) {
      .a = 1, .b = 0, .tx = -centerX,
      .c = 0, .d = 1, .ty = -centerY,
    });
    if (scaleX != 1 || scaleY != 1) {
      AffineMatrix_Concat(&mat, &(AffineMatrix) {
        .a = scaleX, .b = 0,      .tx = 0,
        .c = 0,      .d = scaleY, .ty = 0,
      });
    }
    if (angle != 0) {
      double c = cos(angle);
      double s = sin(angle);
      AffineMatrix_Concat(&mat, &(AffineMatrix) {
        .a = c, .b = -s, .tx = 0,
        .c = s, .d = c,  .ty = 0,
      });
    }
    AffineMatrix_Concat(&mat, &(AffineMatrix) {
      .a = 1, .b = 0, .tx = centerX,
      .c = 0, .d = 1, .ty = centerY,
    });
  }
  AffineMatrix_Concat(&mat, &(AffineMatrix) {
    .a = 1, .b = 0, .tx = NUM2INT(rbX),
    .c = 0, .d = 1, .ty = NUM2INT(rbY),
  });
  if (!AffineMatrix_IsRegular(&mat))
    goto EXIT;

  double dstX00, dstX01, dstX10, dstX11, dstY00, dstY01, dstY10, dstY11;
  AffineMatrix_Transform(&mat, 0,        0,         &dstX00, &dstY00);
  AffineMatrix_Transform(&mat, 0,        srcHeight, &dstX01, &dstY01);
  AffineMatrix_Transform(&mat, srcWidth, 0,         &dstX10, &dstY10);
  AffineMatrix_Transform(&mat, srcWidth, srcHeight, &dstX11, &dstY11);
  double dstX0 = MIN(MIN(MIN(dstX00, dstX01), dstX10), dstX11);
  double dstY0 = MIN(MIN(MIN(dstY00, dstY01), dstY10), dstY11);
  double dstX1 = MAX(MAX(MAX(dstX00, dstX01), dstX10), dstX11);
  double dstY1 = MAX(MAX(MAX(dstY00, dstY01), dstY10), dstY11);
  if (dstTextureWidth <= dstX0 || dstTextureHeight <= dstY0 ||
      dstX1 < 0 || dstY1 < 0)
    goto EXIT;

  AffineMatrix matInv = mat;
  AffineMatrix_Invert(&matInv);
  double srcOX, srcOY;
  AffineMatrix_Transform(&matInv, dstX0 + .5, dstY0 + .5, &srcOX, &srcOY);
  srcOX += srcX;
  srcOY += srcY;
  double srcDXX = matInv.a;
  double srcDXY = matInv.c;
  double srcDYX = matInv.b;
  double srcDYY = matInv.d;

  if (dstX0 < 0) {
    srcOX += -dstX0 * srcDXX;
    srcOY += -dstX0 * srcDXY;
    dstX0 = 0;
  }
  if (dstY0 < 0) {
    srcOX += -dstY0 * srcDYX;
    srcOY += -dstY0 * srcDYY;
    dstY0 = 0;
  }
  int dstX0Int = (int)dstX0;
  int dstY0Int = (int)dstY0;
  int dstWidth  = MIN(dstTextureWidth,  (int)dstX1) - dstX0Int;
  int dstHeight = MIN(dstTextureHeight, (int)dstY1) - dstY0Int;
  
  double srcI;
  double srcJ;
  Pixel* src;
  Pixel* dst = &(dstTexture->pixels[dstX0Int + dstY0Int * dstTextureWidth]);
  for (int j = 0; j < dstHeight;
       j++, dst += -dstWidth + dstTextureWidth) {
    srcI = srcOX + j * srcDYX;
    srcJ = srcOY + j * srcDYY;
    for (int i = 0; i < dstWidth;
         i++, dst++, srcI += srcDXX, srcJ += srcDXY) {
      int srcIInt = (int)floor(srcI);
      int srcJInt = (int)floor(srcJ);
      if (srcIInt < srcX || srcX + srcWidth <= srcIInt ||
          srcJInt < srcY || srcY + srcHeight <= srcJInt)
        continue;
      src = &(srcTexture->pixels[srcIInt + srcJInt * srcTextureWidth]);
      uint8_t srcR = src->color.red;
      uint8_t srcG = src->color.green;
      uint8_t srcB = src->color.blue;
      uint8_t dstR = dst->color.red;
      uint8_t dstG = dst->color.green;
      uint8_t dstB = dst->color.blue;
      uint8_t srcAlpha = DIV255(src->color.alpha * alpha);
      dst->color.alpha = MAX(dst->color.alpha, srcAlpha);
      if (saturation < 255) {
        uint8_t y = 0.30 * srcR + 0.59 * srcG + 0.11 * srcB;
        srcR = ALPHA(srcR, y, saturation);
        srcG = ALPHA(srcG, y, saturation);
        srcB = ALPHA(srcB, y, saturation);
      }
      if (0 < toneRed)
        srcR = ALPHA(255, srcR, toneRed);
      else if (toneRed < 0)
        srcR = ALPHA(0,   srcR, -toneRed);
      if (0 < toneGreen)
        srcG = ALPHA(255, srcG, toneGreen);
      else if (toneGreen < 0)
        srcG = ALPHA(0,   srcG, -toneGreen);
      if (0 < toneBlue)
        srcB = ALPHA(255, srcB, toneBlue);
      else if (toneBlue < 0)
        srcB = ALPHA(0,   srcB, -toneBlue);
      switch (blendType) {
      case ALPHA:
        dstR = ALPHA(srcR, dstR, srcAlpha);
        dstG = ALPHA(srcG, dstG, srcAlpha);
        dstB = ALPHA(srcB, dstB, srcAlpha);
        break;
      case ADD:
        dstR = MIN(255, dstR + DIV255(srcR * srcAlpha));
        dstG = MIN(255, dstG + DIV255(srcG * srcAlpha));
        dstB = MIN(255, dstB + DIV255(srcB * srcAlpha));
        break;
      case SUB:
        dstR = MAX(0, (int)dstR - DIV255(srcR * srcAlpha));
        dstG = MAX(0, (int)dstG - DIV255(srcG * srcAlpha));
        dstB = MAX(0, (int)dstB - DIV255(srcB * srcAlpha));
        break;
      }
      dst->color.red   = dstR;
      dst->color.green = dstG;
      dst->color.blue  = dstB;
    }
  }

EXIT:
  if (!NIL_P(rbClonedTexture))
    Texture_dispose(rbClonedTexture);
  
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
    rb_raise(rb_eArgError, "index out of range: (%d, %d)", x, y);
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
  OBJ_FREEZE(rbSize);
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
  rb_define_method(rb_cTexture, "render_text",    Texture_render_text,    5);
  rb_define_method(rb_cTexture, "render_texture", Texture_render_texture, -1);
  rb_define_method(rb_cTexture, "set_pixel",      Texture_set_pixel,      3);
  rb_define_method(rb_cTexture, "size",           Texture_size,           0);
  rb_define_method(rb_cTexture, "width",          Texture_width,          0);

  symbol_add        = ID2SYM(rb_intern("add"));
  symbol_alpha      = ID2SYM(rb_intern("alpha"));
  symbol_angle      = ID2SYM(rb_intern("angle"));
  symbol_blend_type = ID2SYM(rb_intern("blend_type"));
  symbol_center_x   = ID2SYM(rb_intern("center_x"));
  symbol_center_y   = ID2SYM(rb_intern("center_y"));
  symbol_saturation = ID2SYM(rb_intern("saturation"));
  symbol_scale_x    = ID2SYM(rb_intern("scale_x"));
  symbol_scale_y    = ID2SYM(rb_intern("scale_y"));
  symbol_src_height = ID2SYM(rb_intern("src_height"));
  symbol_src_width  = ID2SYM(rb_intern("src_width"));
  symbol_src_x      = ID2SYM(rb_intern("src_x"));
  symbol_src_y      = ID2SYM(rb_intern("src_y"));
  symbol_sub        = ID2SYM(rb_intern("sub"));
  symbol_tone_blue  = ID2SYM(rb_intern("tone_blue"));
  symbol_tone_green = ID2SYM(rb_intern("tone_green"));
  symbol_tone_red   = ID2SYM(rb_intern("tone_red"));
}
