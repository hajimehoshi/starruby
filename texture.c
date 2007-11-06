#include "starruby.h"

#define DIV255(x) ((x + 255) >> 8)
#define ALPHA(src, dst, a) (DIV255((dst << 8) - dst + (src - dst) * a))

#define macro(x) (x)

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

static SDL_Surface* ConvertSurfaceForScreen(SDL_Surface* surface)
{
  return SDL_ConvertSurface(surface, &(SDL_PixelFormat) {
    .palette = NULL,
    .BitsPerPixel = 32, .BytesPerPixel = 4,
    .Rmask = 0x00ff0000, .Gmask = 0x0000ff00,
    .Bmask = 0x000000ff, .Amask = 0xff000000,
    .Rloss = 0x10, .Gloss = 0x08, .Bloss = 0x00, .Aloss = 0x00,
    .Rshift = 0x00, .Gshift = 0x00, .Bshift = 0x00, .Ashift = 0x08,
    .colorkey = 0, .alpha = 255,
  }, SDL_HWACCEL | SDL_DOUBLEBUF);
}

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
  
  SDL_Surface* surface = ConvertSurfaceForScreen(imageSurface);
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
  SDL_Surface* textSurface = ConvertSurfaceForScreen(textSurfaceRaw);
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

#define RENDER_TEXTURE_LOOP(convertingPixel) \
  for (int j = 0; j < dstHeight;\
       j++, dst += -dstWidth + dstTextureWidth) {\
    int_fast32_t srcI16 = srcOX16 + j * srcDYX16;\
    int_fast32_t srcJ16 = srcOY16 + j * srcDYY16;\
   for (int i = 0; i < dstWidth;\
       i++, dst++, srcI16 += srcDXX16, srcJ16 += srcDXY16) {\
     int_fast32_t srcI = srcI16 >> 16;\
     int_fast32_t srcJ = srcJ16 >> 16;\
     if (srcI < srcX || srcX + srcWidth <= srcI ||\
         srcJ < srcY || srcY + srcHeight <= srcJ)\
       continue;\
     src = &(srcTexture->pixels[srcI + srcJ * srcTextureWidth]);\
     if (src->color.alpha == 0)\
       continue;\
     uint8_t srcR = src->color.red;\
     uint8_t srcG = src->color.green;\
     uint8_t srcB = src->color.blue;\
     uint8_t dstR = dst->color.red;\
     uint8_t dstG = dst->color.green;\
     uint8_t dstB = dst->color.blue;\
     uint8_t srcAlpha = (alpha == 255) ?\
       src->color.alpha : DIV255(src->color.alpha * alpha);\
     dst->color.alpha = MAX(dst->color.alpha, srcAlpha);\
     convertingPixel\
     dst->color.red   = dstR;\
     dst->color.green = dstG;\
     dst->color.blue  = dstB;\
   }\
  }\

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
  
  int srcTextureWidth  = srcTexture->width;
  int srcTextureHeight = srcTexture->height;
  int dstTextureWidth  = dstTexture->width;
  int dstTextureHeight = dstTexture->height;

  int srcX = 0;
  int srcY = 0;
  int srcWidth;
  int srcHeight;
  double scaleX = 1;
  double scaleY = 1;
  double angle = 0;
  int centerX = 0;
  int centerY = 0;
  int alpha = 255;
  BlendType blendType = ALPHA;
  int toneRed   = 0;
  int toneGreen = 0;
  int toneBlue  = 0;
  uint8_t saturation = 255;

  VALUE val;
  st_table* table = RHASH(rbOptions)->tbl;
  if (st_lookup(table, symbol_src_x, &val))
    srcX = NUM2INT(val);
  if (st_lookup(table, symbol_src_y, &val))
    srcY = NUM2INT(val);
  if (st_lookup(table, symbol_src_width, &val))
    srcWidth = NUM2INT(val);
  else
    srcWidth = srcTextureWidth - srcX;
  if (st_lookup(table, symbol_src_height, &val))
    srcHeight = NUM2INT(val);
  else
    srcHeight = srcTextureHeight - srcY;
  if (st_lookup(table, symbol_scale_x, &val))
    scaleX = NUM2DBL(val);
  if (st_lookup(table, symbol_scale_y, &val))
    scaleY = NUM2DBL(val);
  if (st_lookup(table, symbol_angle, &val))
    angle = NUM2DBL(val);
  if (st_lookup(table, symbol_center_x, &val))
    centerX = NUM2INT(val);
  if (st_lookup(table, symbol_center_y, &val))
    centerY = NUM2INT(val);
  if (st_lookup(table, symbol_alpha, &val))
    alpha = NUM2DBL(val);
  if (st_lookup(table, symbol_blend_type, &val)) {
    Check_Type(val, T_SYMBOL);
    if (val == symbol_alpha)
      blendType = ALPHA;
    else if (val == symbol_add)
      blendType = ADD;
    else if (val == symbol_sub)
      blendType = SUB;
  }
  if (st_lookup(table, symbol_tone_red, &val))
    toneRed = NUM2INT(val);
  if (st_lookup(table, symbol_tone_green, &val))
    toneGreen = NUM2INT(val);
  if (st_lookup(table, symbol_tone_blue, &val))
    toneBlue = NUM2INT(val);
  if (st_lookup(table, symbol_saturation, &val))
    saturation = NUM2INT(val);

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
    return Qnil;
  
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
    return Qnil;

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
  
  int_fast32_t srcOX16  = (int)floor(srcOX  * (1 << 16));
  int_fast32_t srcOY16  = (int)floor(srcOY  * (1 << 16));
  int_fast32_t srcDXX16 = (int)floor(srcDXX * (1 << 16));
  int_fast32_t srcDXY16 = (int)floor(srcDXY * (1 << 16));
  int_fast32_t srcDYX16 = (int)floor(srcDYX * (1 << 16));
  int_fast32_t srcDYY16 = (int)floor(srcDYY * (1 << 16));
  Pixel* src;
  Pixel* dst = &(dstTexture->pixels[dstX0Int + dstY0Int * dstTextureWidth]);
  
  VALUE rbClonedTexture = Qnil;
  if (self == rbTexture) {
    rbClonedTexture = rb_funcall(rbTexture, rb_intern("clone"), 0);
    Data_Get_Struct(rbClonedTexture, Texture, srcTexture);
  }

  if (saturation == 255 &&
      toneRed == 0 && toneGreen == 0 && toneBlue == 0 &&
      blendType == ALPHA) {
    RENDER_TEXTURE_LOOP({
      dstR = ALPHA(srcR, dstR, srcAlpha);
      dstG = ALPHA(srcG, dstG, srcAlpha);
      dstB = ALPHA(srcB, dstB, srcAlpha);
    });
  } else {
    RENDER_TEXTURE_LOOP({
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
    });
  }

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
