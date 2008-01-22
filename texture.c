#include "starruby.h"
#include <png.h>

#define ALPHA(src, dst, a) (a == 255 ? src :\
                            (a == 0 ? dst :\
                             DIV255((dst << 8) - dst + (src - dst) * a)))

volatile static VALUE symbol_add;
volatile static VALUE symbol_alpha;
volatile static VALUE symbol_angle;
volatile static VALUE symbol_blend_type;
volatile static VALUE symbol_camera_angle;
volatile static VALUE symbol_camera_height;
volatile static VALUE symbol_camera_x;
volatile static VALUE symbol_camera_y;
volatile static VALUE symbol_center_x;
volatile static VALUE symbol_center_y;
volatile static VALUE symbol_distance;
volatile static VALUE symbol_horizontal;
volatile static VALUE symbol_intersection_x;
volatile static VALUE symbol_intersection_y;
volatile static VALUE symbol_loop;
volatile static VALUE symbol_saturation;
volatile static VALUE symbol_scale_x;
volatile static VALUE symbol_scale_y;
volatile static VALUE symbol_src_height;
volatile static VALUE symbol_src_width;
volatile static VALUE symbol_src_x;
volatile static VALUE symbol_src_y;
volatile static VALUE symbol_sub;
volatile static VALUE symbol_tone_blue;
volatile static VALUE symbol_tone_green;
volatile static VALUE symbol_tone_red;
volatile static VALUE symbol_vanishing_x;
volatile static VALUE symbol_vanishing_y;
volatile static VALUE symbol_vertical;

typedef enum {
  ALPHA,
  ADD,
  SUB,
} BlendType;

static SDL_Surface*
ConvertSurfaceForScreen(SDL_Surface* surface)
{
  return SDL_ConvertSurface(surface, &(SDL_PixelFormat) {
    .palette = NULL,
    .BitsPerPixel = 32, .BytesPerPixel = 4,
    .Rmask = 0x00ff0000, .Gmask = 0x0000ff00,
    .Bmask = 0x000000ff, .Amask = 0xff000000,
    .colorkey = 0, .alpha = 255,
  }, SDL_HWACCEL | SDL_DOUBLEBUF);
}

static VALUE
Texture_load(VALUE self, VALUE rbPath)
{
  volatile VALUE rbCompletePath = GetCompletePath(rbPath, true);
  char* path = StringValuePtr(rbCompletePath);
  FILE* fp = fopen(path, "rb");
  png_byte header[8];
  fread(&header, 1, 8, fp);
  if (png_sig_cmp(header, 0, 8)) {
    rb_raise(rb_eStarRubyError, "invalid PNG file: %s", path);
    return Qnil;
  }
  png_structp pngPtr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
                                              NULL, NULL, NULL);
  if (!pngPtr) {
    fclose(fp);
    rb_raise(rb_eStarRubyError, "PNG error: %s", path);
    return Qnil;
  }
  png_infop infoPtr = png_create_info_struct(pngPtr);
  if (!infoPtr) {
    png_destroy_read_struct(&pngPtr, NULL, NULL);
    fclose(fp);
    rb_raise(rb_eStarRubyError, "PNG error: %s", path);
    return Qnil;
  }
  png_infop endInfo = png_create_info_struct(pngPtr);
  if (!endInfo) {
    png_destroy_read_struct(&pngPtr, &infoPtr, NULL);
    fclose(fp);
    rb_raise(rb_eStarRubyError, "PNG error: %s", path);
    return Qnil;
  }

  png_init_io(pngPtr, fp);
  png_set_sig_bytes(pngPtr, 8);
  png_read_info(pngPtr, infoPtr);
  png_uint_32 width, height;
  int bitDepth, colorType, interlaceType;
  png_get_IHDR(pngPtr, infoPtr, &width, &height,
               &bitDepth, &colorType, &interlaceType, NULL, NULL);
  if (interlaceType != PNG_INTERLACE_NONE) {
    png_destroy_read_struct(&pngPtr, &infoPtr, &endInfo);
    fclose(fp);
    rb_raise(rb_eStarRubyError,
             "not supported interlacing PNG image: %s", path);
    return Qnil;
  }
  if (colorType == PNG_COLOR_TYPE_PALETTE)
    png_set_palette_to_rgb(pngPtr);
  if (colorType == PNG_COLOR_TYPE_GRAY && bitDepth < 8)
    png_set_gray_1_2_4_to_8(pngPtr);
  if (png_get_valid(pngPtr, infoPtr, PNG_INFO_tRNS))
    png_set_tRNS_to_alpha(pngPtr);
  png_read_update_info(pngPtr, infoPtr);
  volatile VALUE rbTexture = rb_funcall(self, rb_intern("new"), 2,
                                        INT2NUM(width), INT2NUM(height));
  Texture* texture;
  Data_Get_Struct(rbTexture, Texture, texture);

  int channels = png_get_channels(pngPtr, infoPtr);
  for (int j = 0; j < height; j++) {
    png_byte row[width * channels];
    png_read_row(pngPtr, row, NULL);
    for (int i = 0; i < width; i++) {
      Color* c = &(texture->pixels[width * j + i].color);
      switch (channels) {
      case 2:
        c->red = c->green = c->blue = row[i * channels];
        c->alpha = row[i * channels + 1];
        break;
      case 3:
      case 4:
        c->red   = row[i * channels];
        c->green = row[i * channels + 1];
        c->blue  = row[i * channels + 2];
        c->alpha = (channels == 4) ? row[i * channels + 3] : 0xff;
        break;
      }
    }
  }
  png_read_end(pngPtr, endInfo);
  png_destroy_read_struct(&pngPtr, &infoPtr, &endInfo);
  fclose(fp);

  return rbTexture;
}

static void
Texture_free(Texture* texture)
{
  free(texture->pixels);
  texture->pixels = NULL;
  free(texture);
}

static VALUE
Texture_alloc(VALUE klass)
{
  Texture* texture = ALLOC(Texture);
  texture->pixels = NULL;
  return Data_Wrap_Struct(klass, 0, Texture_free, texture);
}

static VALUE
Texture_initialize(VALUE self, VALUE rbWidth, VALUE rbHeight)
{
  Texture* texture;
  Data_Get_Struct(self, Texture, texture);
  int width  = NUM2INT(rbWidth);
  int height = NUM2INT(rbHeight);
  if (width <= 0) {
    rb_raise(rb_eArgError, "width less than or equal to 0");
    return Qnil;
  }
  if (height <= 0) {
    rb_raise(rb_eArgError, "height less than or equal to 0");
    return Qnil;
  }
  texture->width  = width;
  texture->height = height;
  texture->pixels = ALLOC_N(Pixel, texture->width * texture->height);
  MEMZERO(texture->pixels, Pixel, texture->width * texture->height);
  return Qnil;
}

static VALUE
Texture_initialize_copy(VALUE self, VALUE rbTexture)
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

typedef struct {
  int cameraX;
  int cameraY;
  int cameraHeight;
  double cameraAngleVertical;
  double cameraAngleHorizontal;
  double distance;
  int intersectionX;
  int intersectionY;
  bool isLoop;
} PerspectiveOptions;

static void
AssignPerspectiveOptions(PerspectiveOptions* options, VALUE rbOptions)
{
  volatile VALUE val;
  Check_Type(rbOptions, T_HASH);
  MEMZERO(options, PerspectiveOptions, 1);
  if (!NIL_P(val = rb_hash_aref(rbOptions, symbol_camera_x)))
    options->cameraX = NUM2INT(val);
  if (!NIL_P(val = rb_hash_aref(rbOptions, symbol_camera_y)))
    options->cameraY = NUM2INT(val);
  if (!NIL_P(val = rb_hash_aref(rbOptions, symbol_camera_height)))
    options->cameraHeight = NUM2DBL(val);
  if (!NIL_P(val = rb_hash_aref(rbOptions, symbol_camera_angle))) {
    switch (TYPE(val)) {
    case T_FIXNUM:
    case T_BIGNUM:
    case T_FLOAT:
      rb_warn("Numeric object for :camera_angle is deprecated");
      options->cameraAngleVertical = NUM2DBL(val);
      break;
    case T_HASH:
      {
        volatile VALUE rbH = val;
        if (!NIL_P(val = rb_hash_aref(rbH, symbol_vertical)))
          options->cameraAngleVertical = NUM2DBL(val);
        if (!NIL_P(val = rb_hash_aref(rbH, symbol_horizontal)))
          options->cameraAngleHorizontal = NUM2DBL(val);
      }
      break;
    default:
      Check_Type(val, T_HASH);
      break;
    }
  }
  if (!NIL_P(val = rb_hash_aref(rbOptions, symbol_distance)))
    options->distance = NUM2DBL(val);
  if (!NIL_P(val = rb_hash_aref(rbOptions, symbol_vanishing_x))) {
    rb_warn(":vanishing_x is desprecated; use :intersection_x instead");
    options->intersectionX = NUM2INT(val);
  }
  if (!NIL_P(val = rb_hash_aref(rbOptions, symbol_vanishing_y))) {
    rb_warn(":vanishing_y is desprecated; use :intersection_y instead");
    options->intersectionY = NUM2INT(val);
  }
  if (!NIL_P(val = rb_hash_aref(rbOptions, symbol_intersection_x)))
    options->intersectionX = NUM2INT(val);
  if (!NIL_P(val = rb_hash_aref(rbOptions, symbol_intersection_y)))
    options->intersectionY = NUM2INT(val);
  if (!NIL_P(val = rb_hash_aref(rbOptions, symbol_loop)))
    options->isLoop = RTEST(val);
}

static VALUE
Texture_transform_in_perspective(int argc, VALUE* argv, VALUE self)
{
  volatile VALUE rbX, rbY, rbHeight, rbOptions;
  rb_scan_args(argc, argv, "31", &rbX, &rbY, &rbHeight, &rbOptions);
  if (NIL_P(rbOptions))
    rbOptions = rb_hash_new();
  int x = NUM2INT(rbX);
  int y = NUM2INT(rbY);
  double height = NUM2DBL(rbHeight);
  PerspectiveOptions options;
  AssignPerspectiveOptions(&options, rbOptions);
  double cosAngle = cos(options.cameraAngleVertical);
  double sinAngle = sin(options.cameraAngleVertical);
  double xInPSystem = cosAngle  * (x - options.cameraX)
    + sinAngle * (y - options.cameraY);
  double zInPSystem = -sinAngle * (x - options.cameraX)
    + cosAngle * (y - options.cameraY);
  volatile VALUE rbResult = rb_ary_new3(3, Qnil, Qnil, Qnil);
  OBJ_FREEZE(rbResult);
  if (zInPSystem == 0)
    return rbResult;
  double scale = -options.distance / zInPSystem;
  int newX = (int)(xInPSystem * scale + options.intersectionX);
  int newY = (int)((options.cameraHeight - height) * scale
                   + options.intersectionY);
  RARRAY_PTR(rbResult)[0] = FIXABLE(newX) ? INT2FIX(newX) : Qnil;
  RARRAY_PTR(rbResult)[1] = FIXABLE(newY) ? INT2FIX(newY) : Qnil;
  RARRAY_PTR(rbResult)[2] = rb_float_new(scale);
  return rbResult;
}

static VALUE Texture_change_hue_bang(VALUE, VALUE);
static VALUE
Texture_change_hue(VALUE self, VALUE rbAngle)
{
  Texture* texture;
  Data_Get_Struct(self, Texture, texture);
  if (!texture->pixels) {
    rb_raise(rb_eRuntimeError, "can't modify disposed texture");
    return Qnil;
  }

  volatile VALUE rbTexture = rb_funcall(self, rb_intern("dup"), 0);
  Texture_change_hue_bang(rbTexture, rbAngle);
  return rbTexture;
}

static VALUE
Texture_change_hue_bang(VALUE self, VALUE rbAngle)
{
  rb_check_frozen(self);

  Texture* texture;
  Data_Get_Struct(self, Texture, texture);
  if (!texture->pixels) {
    rb_raise(rb_eRuntimeError, "can't modify disposed texture");
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
    if (max != 0) {
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
      int ii = (int)h;
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
  }
  return Qnil;
}

static VALUE
Texture_clear(VALUE self)
{
  rb_check_frozen(self);
  Texture* texture;
  Data_Get_Struct(self, Texture, texture);
  if (!texture->pixels) {
    rb_raise(rb_eRuntimeError, "can't modify disposed texture");
    return Qnil;
  }
  MEMZERO(texture->pixels, Color, texture->width * texture->height);
  return Qnil;
}

static VALUE
Texture_dispose(VALUE self)
{
  Texture* texture;
  Data_Get_Struct(self, Texture, texture);
  free(texture->pixels);
  texture->pixels = NULL;
  return Qnil;
}

static VALUE
Texture_disposed(VALUE self)
{
  Texture* texture;
  Data_Get_Struct(self, Texture, texture);
  return !texture->pixels ? Qtrue : Qfalse;
}

static VALUE
Texture_dump(VALUE self, VALUE rbFormat)
{
  Texture* texture;
  Data_Get_Struct(self, Texture, texture);
  if (!texture->pixels) {
    rb_raise(rb_eRuntimeError, "can't modify disposed texture");
    return Qnil;
  }

  char* format = StringValuePtr(rbFormat);
  int textureSize = texture->width * texture->height;
  int formatLength = RSTRING_LEN(rbFormat);
  volatile VALUE rbResult = rb_str_new(NULL, textureSize * formatLength);
  uint8_t* strPtr = (uint8_t*)RSTRING_PTR(rbResult);
  Pixel* pixels = texture->pixels;
  for (int i = 0; i < textureSize; i++, pixels++) {
    for (int j = 0; j < formatLength; j++, strPtr++) {
      switch (format[j]) {
      case 'r': *strPtr = pixels->color.red;   break;
      case 'g': *strPtr = pixels->color.green; break;
      case 'b': *strPtr = pixels->color.blue;  break;
      case 'a': *strPtr = pixels->color.alpha; break;
      }
    }
  }
  return rbResult;
}

static VALUE
Texture_fill(VALUE self, VALUE rbColor)
{
  rb_check_frozen(self);
  Texture* texture;
  Data_Get_Struct(self, Texture, texture);
  if (!texture->pixels) {
    rb_raise(rb_eRuntimeError, "can't modify disposed texture");
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

static VALUE
Texture_fill_rect(VALUE self, VALUE rbX, VALUE rbY,
                  VALUE rbWidth, VALUE rbHeight, VALUE rbColor)
{
  rb_check_frozen(self);
  Texture* texture;
  Data_Get_Struct(self, Texture, texture);
  if (!texture->pixels) {
    rb_raise(rb_eRuntimeError, "can't modify disposed texture");
    return Qnil;
  }
  int rectX = NUM2INT(rbX);
  int rectY = NUM2INT(rbY);
  int rectWidth  = NUM2INT(rbWidth);
  int rectHeight = NUM2INT(rbHeight);
  Color* color;
  Data_Get_Struct(rbColor, Color, color);
  int width = texture->width;
  Pixel* pixels = texture->pixels;
  for (int j = rectY; j < rectY + rectHeight; j++)
    for (int i = rectX; i < rectX + rectWidth; i++)
      pixels[i + j * width].color = *color;
  return Qnil;
}

static VALUE
Texture_get_pixel(VALUE self, VALUE rbX, VALUE rbY)
{
  int x = NUM2INT(rbX);
  int y = NUM2INT(rbY);
  Texture* texture;
  Data_Get_Struct(self, Texture, texture);
  if (!texture->pixels) {
    rb_raise(rb_eRuntimeError, "can't modify disposed texture");
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

static VALUE
Texture_height(VALUE self)
{
  Texture* texture;
  Data_Get_Struct(self, Texture, texture);
  if (!texture->pixels) {
    rb_raise(rb_eRuntimeError, "can't modify disposed texture");
    return Qnil;
  }
  return INT2NUM(texture->height);
}

static VALUE
Texture_render_in_perspective(int argc, VALUE* argv, VALUE self)
{
  rb_check_frozen(self);
  volatile VALUE rbTexture, rbOptions;
  rb_scan_args(argc, argv, "11", &rbTexture, &rbOptions);
  if (NIL_P(rbOptions))
    rbOptions = rb_hash_new();
  Texture* srcTexture;
  Data_Get_Struct(rbTexture, Texture, srcTexture);
  if (!srcTexture->pixels) {
    rb_raise(rb_eRuntimeError, "can't use disposed texture");
    return Qnil;
  }
  Texture* dstTexture;
  Data_Get_Struct(self, Texture, dstTexture);
  if (!dstTexture->pixels) {
    rb_raise(rb_eRuntimeError, "can't use disposed texture");
    return Qnil;
  }
  if (srcTexture == dstTexture) {
    rb_raise(rb_eRuntimeError, "can't render self in perspective");
    return Qnil;
  }
  PerspectiveOptions options;
  AssignPerspectiveOptions(&options, rbOptions);
  if (options.cameraHeight == 0)
    return Qnil;
  int srcWidth  = srcTexture->width;
  int srcHeight = srcTexture->height;
  int dstWidth  = dstTexture->width;
  int dstHeight = dstTexture->height;
  Pixel* src = srcTexture->pixels;
  Pixel* dst = dstTexture->pixels;
  double cosAngle = cos(options.cameraAngleVertical);
  double sinAngle = sin(options.cameraAngleVertical);
  int screenTop    = options.cameraHeight + options.intersectionY;
  int screenBottom = screenTop - dstHeight;
  int screenLeft   = -options.intersectionX;
  int screenRight  = screenLeft + dstWidth;
  for (int j = screenTop - 1; screenBottom <= j; j--) {
    double dHeight = options.cameraHeight - j;
    if ((0 < options.cameraHeight && (dHeight <= 0)) ||
        (options.cameraHeight < 0 && (0 <= dHeight))) {
      dst += dstWidth;
    } else {
      double scale = options.cameraHeight / dHeight;
      double srcZInPSystem = -options.distance * scale;
      for (int i = screenLeft; i < screenRight; i++, dst++) {
        double srcXInPSystem = i * scale;
        int srcX = (int)(cosAngle * srcXInPSystem - sinAngle * srcZInPSystem
                         + options.cameraX);
        int srcY = (int)(sinAngle * srcXInPSystem + cosAngle * srcZInPSystem
                         + options.cameraY);
        if (options.isLoop) {
          srcX %= srcWidth;
          if (srcX < 0)
            srcX += srcWidth;
          srcY %= srcHeight;
          if (srcY < 0)
            srcY += srcHeight;
        }
        if (options.isLoop ||
            (0 <= srcX && srcX < srcWidth && 0 <= srcY && srcY < srcHeight)) {
          Color* srcColor = &(src[srcX + srcY * srcWidth].color);
          uint8_t alpha = (dst->color.alpha == 0) ? 255 : srcColor->alpha;
          dst->color.red   = ALPHA(srcColor->red,   dst->color.red,   alpha);
          dst->color.green = ALPHA(srcColor->green, dst->color.green, alpha);
          dst->color.blue  = ALPHA(srcColor->blue,  dst->color.blue,  alpha);
          dst->color.alpha = MAX(dst->color.alpha, src->color.alpha);
        }
      }
    }
  }
  return Qnil;
}

static VALUE Texture_render_texture(int, VALUE*, VALUE);
static VALUE
Texture_render_text(int argc, VALUE* argv, VALUE self)
{
  volatile VALUE rbText, rbX, rbY, rbFont, rbColor, rbAntiAlias;
  rb_scan_args(argc, argv, "51",
               &rbText, &rbX, &rbY, &rbFont, &rbColor, &rbAntiAlias);
  Check_Type(rbText, T_STRING);
  if (!(RSTRING_LEN(rbText)))
    return Qnil;
  bool antiAlias = RTEST(rbAntiAlias);
  Check_Type(rbText, T_STRING);
  if (!RSTRING_LEN(rbText)) {
    rb_raise(rb_eArgError, "empty text");
    return Qnil;
  }
  char* text = StringValuePtr(rbText);
  Font* font;
  Data_Get_Struct(rbFont, Font, font);
  if (!font->sdlFont) {
    rb_raise(rb_eRuntimeError, "can't use disposed font");
    return Qnil;
  }
  volatile VALUE rbSize = rb_funcall(rbFont, rb_intern("get_size"), 1, rbText);
  volatile VALUE rbTextTexture = rb_funcall2(rb_cTexture, rb_intern("new"),
                                             2, RARRAY_PTR(rbSize));
  Texture* textTexture;
  Data_Get_Struct(rbTextTexture, Texture, textTexture);
  Color* color;
  Data_Get_Struct(rbColor, Color, color);

  SDL_Surface* textSurfaceRaw;
  if (antiAlias)
    textSurfaceRaw = TTF_RenderUTF8_Shaded(font->sdlFont, text,
                                           (SDL_Color) {255, 255, 255, 255},
                                           (SDL_Color) {0, 0, 0, 0});
  else
    textSurfaceRaw = TTF_RenderUTF8_Solid(font->sdlFont, text,
                                          (SDL_Color) {255, 255, 255, 255});
  if (!textSurfaceRaw) {
    rb_raise_sdl_ttf_error();
    return Qnil;
  }
  SDL_Surface* textSurface = ConvertSurfaceForScreen(textSurfaceRaw);
  SDL_FreeSurface(textSurfaceRaw);
  textSurfaceRaw = NULL;
  if (!textSurface) {
    rb_raise_sdl_error();
    return Qnil;
  }
  SDL_LockSurface(textSurface);
  Pixel* src = (Pixel*)(textSurface->pixels);
  Pixel* dst = textTexture->pixels;
  int size = textTexture->width * textTexture->height;
  for (int i = 0; i < size; i++, src++, dst++) {
    if (src->value) {
      dst->color = *color;
      dst->color.alpha = DIV255(src->color.red * color->alpha);
    }
  }
  SDL_UnlockSurface(textSurface);
  SDL_FreeSurface(textSurface);
  textSurface = NULL;

  Texture_render_texture(3, (VALUE[]) {rbTextTexture, rbX, rbY}, self);
  Texture_dispose(rbTextTexture);
  return Qnil;
}

#define RENDER_TEXTURE_LOOP(convertingPixel) do {\
  int srcX2 = srcX + srcWidth;\
  int srcY2 = srcY + srcHeight;\
  for (int j = 0; j < dstHeight;\
       j++, dst += -dstWidth + dstTextureWidth) {\
    int_fast32_t srcI16 = srcOX16 + j * srcDYX16;\
    int_fast32_t srcJ16 = srcOY16 + j * srcDYY16;\
    for (int i = 0; i < dstWidth;\
         i++, dst++, srcI16 += srcDXX16, srcJ16 += srcDXY16) {\
      int_fast32_t srcI = srcI16 >> 16;\
      int_fast32_t srcJ = srcJ16 >> 16;\
      if (srcX <= srcI && srcI < srcX2 && srcY <= srcJ && srcJ < srcY2) {\
        src = &(srcTexture->pixels[srcI + srcJ * srcTextureWidth]);\
        uint8_t srcR = src->color.red;\
        uint8_t srcG = src->color.green;\
        uint8_t srcB = src->color.blue;\
        uint8_t dstR = dst->color.red;\
        uint8_t dstG = dst->color.green;\
        uint8_t dstB = dst->color.blue;\
        uint8_t srcAlpha = DIV255(src->color.alpha * alpha);\
        uint8_t pixelAlpha = (dst->color.alpha == 0) ? 255 : srcAlpha;\
        dst->color.alpha = MAX(dst->color.alpha, srcAlpha);\
        convertingPixel;\
        dst->color.red   = dstR;\
        dst->color.green = dstG;\
        dst->color.blue  = dstB;\
      }\
    }\
  }\
} while (false)\

static VALUE
Texture_render_texture(int argc, VALUE* argv, VALUE self)
{
  rb_check_frozen(self);
  Texture* dstTexture;
  Data_Get_Struct(self, Texture, dstTexture);
  if (!dstTexture->pixels) {
    rb_raise(rb_eRuntimeError, "can't modify disposed texture");
    return Qnil;
  }

  volatile VALUE rbTexture, rbX, rbY, rbOptions;
  rb_scan_args(argc, argv, "31", &rbTexture, &rbX, &rbY, &rbOptions);
  if (NIL_P(rbOptions))
    rbOptions = rb_hash_new();

  Texture* srcTexture;
  Data_Get_Struct(rbTexture, Texture, srcTexture);
  if (!srcTexture->pixels) {
    rb_raise(rb_eRuntimeError, "can't use disposed texture");
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

  volatile VALUE val;
  Check_Type(rbOptions, T_HASH);
  if (!NIL_P(val = rb_hash_aref(rbOptions, symbol_src_x)))
    srcX = NUM2INT(val);
  if (!NIL_P(val = rb_hash_aref(rbOptions, symbol_src_y)))
    srcY = NUM2INT(val);
  if (!NIL_P(val = rb_hash_aref(rbOptions, symbol_src_width)))
    srcWidth = NUM2INT(val);
  else
    srcWidth = srcTextureWidth - srcX;
  if (!NIL_P(val = rb_hash_aref(rbOptions, symbol_src_height)))
    srcHeight = NUM2INT(val);
  else
    srcHeight = srcTextureHeight - srcY;
  if (!NIL_P(val = rb_hash_aref(rbOptions, symbol_scale_x)))
    scaleX = NUM2DBL(val);
  if (!NIL_P(val = rb_hash_aref(rbOptions, symbol_scale_y)))
    scaleY = NUM2DBL(val);
  if (!NIL_P(val = rb_hash_aref(rbOptions, symbol_angle)))
    angle = NUM2DBL(val);
  if (!NIL_P(val = rb_hash_aref(rbOptions, symbol_center_x)))
    centerX = NUM2INT(val);
  if (!NIL_P(val = rb_hash_aref(rbOptions, symbol_center_y)))
    centerY = NUM2INT(val);
  if (!NIL_P(val = rb_hash_aref(rbOptions, symbol_alpha)))
    alpha = NUM2DBL(val);
  if (!NIL_P(val = rb_hash_aref(rbOptions, symbol_blend_type))) {
    Check_Type(val, T_SYMBOL);
    if (val == symbol_alpha)
      blendType = ALPHA;
    else if (val == symbol_add)
      blendType = ADD;
    else if (val == symbol_sub)
      blendType = SUB;
  }
  if (!NIL_P(val = rb_hash_aref(rbOptions, symbol_tone_red)))
    toneRed = NUM2INT(val);
  if (!NIL_P(val = rb_hash_aref(rbOptions, symbol_tone_green)))
    toneGreen = NUM2INT(val);
  if (!NIL_P(val = rb_hash_aref(rbOptions, symbol_tone_blue)))
    toneBlue = NUM2INT(val);
  if (!NIL_P(val = rb_hash_aref(rbOptions, symbol_saturation)))
    saturation = NUM2INT(val);

  if (!(0 <= srcX && srcX < srcTexture->width)) {
    rb_raise(rb_eArgError, "invalid src_x: %d", srcX);
    return Qnil;
  }
  if (!(0 <= srcY && srcY < srcTexture->height)) {
    rb_raise(rb_eArgError, "invalid src_y: %d", srcY);
    return Qnil;
  }
  if (!(0 <= srcWidth && srcWidth <= srcTexture->width - srcX)) {
    rb_raise(rb_eArgError, "invalid src_width: %d", srcWidth);
    return Qnil;
  }
  if (!(0 <= srcHeight && srcHeight <= srcTexture->height - srcY)) {
    rb_raise(rb_eArgError, "invalid src_height: %d", srcHeight);
    return Qnil;
  }

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
  AffineMatrix_Transform(&matInv, dstX0 + 0.5, dstY0 + 0.5, &srcOX, &srcOY);
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

  int_fast32_t srcOX16  = (int)(srcOX  * (1 << 16));
  int_fast32_t srcOY16  = (int)(srcOY  * (1 << 16));
  int_fast32_t srcDXX16 = (int)(srcDXX * (1 << 16));
  int_fast32_t srcDXY16 = (int)(srcDXY * (1 << 16));
  int_fast32_t srcDYX16 = (int)(srcDYX * (1 << 16));
  int_fast32_t srcDYY16 = (int)(srcDYY * (1 << 16));
  Pixel* src;
  Pixel* dst = &(dstTexture->pixels[dstX0Int + dstY0Int * dstTextureWidth]);

  volatile VALUE rbClonedTexture = Qnil;
  if (self == rbTexture) {
    rbClonedTexture = rb_funcall(rbTexture, rb_intern("clone"), 0);
    Data_Get_Struct(rbClonedTexture, Texture, srcTexture);
  }

  if (saturation == 255 &&
      toneRed == 0 && toneGreen == 0 && toneBlue == 0 &&
      blendType == ALPHA) {
    RENDER_TEXTURE_LOOP({
      dstR = ALPHA(srcR, dstR, pixelAlpha);
      dstG = ALPHA(srcG, dstG, pixelAlpha);
      dstB = ALPHA(srcB, dstB, pixelAlpha);
    });
  } else {
    RENDER_TEXTURE_LOOP({
      if (saturation < 255) {
        // http://www.poynton.com/ColorFAQ.html
        uint8_t y = (6969 * srcR + 23434 * srcG + 2365 * srcB) / 32768;
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
        dstR = ALPHA(srcR, dstR, pixelAlpha);
        dstG = ALPHA(srcG, dstG, pixelAlpha);
        dstB = ALPHA(srcB, dstB, pixelAlpha);
        break;
      case ADD:
        dstR = MIN(255, dstR + DIV255(srcR * pixelAlpha));
        dstG = MIN(255, dstG + DIV255(srcG * pixelAlpha));
        dstB = MIN(255, dstB + DIV255(srcB * pixelAlpha));
        break;
      case SUB:
        dstR = MAX(0, (int)dstR - DIV255(srcR * pixelAlpha));
        dstG = MAX(0, (int)dstG - DIV255(srcG * pixelAlpha));
        dstB = MAX(0, (int)dstB - DIV255(srcB * pixelAlpha));
        break;
      }
    });
  }

  if (!NIL_P(rbClonedTexture))
    Texture_dispose(rbClonedTexture);

  return Qnil;
}

static VALUE
Texture_save(int argc, VALUE* argv, VALUE self)
{
  Texture* texture;
  Data_Get_Struct(self, Texture, texture);
  if (!texture->pixels) {
    rb_raise(rb_eRuntimeError, "can't modify disposed texture");
    return Qnil;
  }
  volatile VALUE rbPath, rbAlpha;
  rb_scan_args(argc, argv, "11", &rbPath, &rbAlpha);
  if (argc == 1)
    rbAlpha = Qtrue;
  char* path = StringValuePtr(rbPath);
  FILE* fp = fopen(path, "wb");
  if (!fp) {
    rb_raise(rb_path2class("Errno::ENOENT"), "%s", path);
    return Qnil;
  }
  png_structp pngPtr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
                                               NULL, NULL, NULL);
  png_infop infoPtr = png_create_info_struct(pngPtr);
  png_init_io(pngPtr, fp);
  png_set_IHDR(pngPtr, infoPtr, texture->width, texture->height,
               8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
               PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
  png_write_info(pngPtr, infoPtr);
  for (int j = 0; j < texture->height; j++) {
    png_byte row[texture->width * 4];
    for (int i = 0; i < texture->width; i++) {
      Color* c = &(texture->pixels[texture->width * j + i].color);
      row[i * 4]     = c->red;
      row[i * 4 + 1] = c->green;
      row[i * 4 + 2] = c->blue;
      if (RTEST(rbAlpha))
        row[i * 4 + 3] = c->alpha;
      else
        row[i * 4 + 3] = 0xff;
    }
    png_write_row(pngPtr, row);
  }
  png_write_end(pngPtr, infoPtr);
  png_destroy_write_struct(&pngPtr, &infoPtr);
  fclose(fp);
  return Qnil;
}

static VALUE
Texture_set_pixel(VALUE self, VALUE rbX, VALUE rbY, VALUE rbColor)
{
  rb_check_frozen(self);
  Texture* texture;
  Data_Get_Struct(self, Texture, texture);
  if (!texture->pixels) {
    rb_raise(rb_eRuntimeError, "can't modify disposed texture");
    return Qnil;
  }
  int x = NUM2INT(rbX), y = NUM2INT(rbY);
  if (x < 0 || texture->width <= x || y < 0 || texture->height <= y) {
    rb_raise(rb_eArgError, "index out of range: (%d, %d)", x, y);
    return Qnil;
  }
  Color* color;
  Data_Get_Struct(rbColor, Color, color);
  texture->pixels[x + y * texture->width].color = *color;
  return rbColor;
}

static VALUE
Texture_size(VALUE self)
{
  Texture* texture;
  Data_Get_Struct(self, Texture, texture);
  if (!texture->pixels) {
    rb_raise(rb_eRuntimeError, "can't modify disposed texture");
    return Qnil;
  }
  volatile VALUE rbSize = rb_assoc_new(INT2NUM(texture->width),
                                       INT2NUM(texture->height));
  OBJ_FREEZE(rbSize);
  return rbSize;
}

static VALUE
Texture_undump(VALUE self, VALUE rbData, VALUE rbFormat)
{
  rb_check_frozen(self);
  Texture* texture;
  Data_Get_Struct(self, Texture, texture);
  if (!texture->pixels) {
    rb_raise(rb_eRuntimeError, "can't modify disposed texture");
    return Qnil;
  }
  char* format = StringValuePtr(rbFormat);
  int formatLength = RSTRING_LEN(rbFormat);
  int textureSize = texture->width * texture->height;
  Check_Type(rbData, T_STRING);
  if (textureSize * formatLength != RSTRING_LEN(rbData)) {
    rb_raise(rb_eArgError, "invalid data size: %d expected but was %ld",             textureSize * formatLength, RSTRING_LEN(rbData));
    return Qnil;
  }
  uint8_t* data = (uint8_t*)RSTRING_PTR(rbData);
  Pixel* pixels = texture->pixels;
  for (int i = 0; i < textureSize; i++, pixels++) {
    for (int j = 0; j < formatLength; j++, data++) {
      switch (format[j]) {
      case 'r': pixels->color.red   = *data; break;
      case 'g': pixels->color.green = *data; break;
      case 'b': pixels->color.blue  = *data; break;
      case 'a': pixels->color.alpha = *data; break;
      }
    }
  }
  return Qnil;
}

static VALUE
Texture_width(VALUE self)
{
  Texture* texture;
  Data_Get_Struct(self, Texture, texture);
  if (!texture->pixels) {
    rb_raise(rb_eRuntimeError, "can't modify disposed texture");
    return Qnil;
  }
  return INT2NUM(texture->width);
}

void
InitializeTexture(void)
{
  rb_cTexture = rb_define_class_under(rb_mStarRuby, "Texture", rb_cObject);
  rb_define_singleton_method(rb_cTexture, "load",     Texture_load,     1);
  rb_define_alloc_func(rb_cTexture, Texture_alloc);
  rb_define_private_method(rb_cTexture, "initialize", Texture_initialize, 2);
  rb_define_private_method(rb_cTexture, "initialize_copy",
                           Texture_initialize_copy, 1);
  rb_define_singleton_method(rb_cTexture, "transform_in_perspective",
                             Texture_transform_in_perspective, -1);
  rb_define_method(rb_cTexture, "change_hue",     Texture_change_hue,      1);
  rb_define_method(rb_cTexture, "change_hue!",    Texture_change_hue_bang, 1);
  rb_define_method(rb_cTexture, "clear",          Texture_clear,           0);
  rb_define_method(rb_cTexture, "dispose",        Texture_dispose,         0);
  rb_define_method(rb_cTexture, "disposed?",      Texture_disposed,        0);
  rb_define_method(rb_cTexture, "render_in_perspective",
                   Texture_render_in_perspective, -1);
  rb_define_method(rb_cTexture, "dump",           Texture_dump,            1);
  rb_define_method(rb_cTexture, "fill",           Texture_fill,            1);
  rb_define_method(rb_cTexture, "fill_rect",      Texture_fill_rect,       5);
  rb_define_method(rb_cTexture, "get_pixel",      Texture_get_pixel,       2);
  rb_define_method(rb_cTexture, "height",         Texture_height,          0);
  rb_define_method(rb_cTexture, "render_text",    Texture_render_text,     -1);
  rb_define_method(rb_cTexture, "render_texture", Texture_render_texture,  -1);
  rb_define_method(rb_cTexture, "save",           Texture_save,            -1);
  rb_define_method(rb_cTexture, "set_pixel",      Texture_set_pixel,       3);
  rb_define_method(rb_cTexture, "size",           Texture_size,            0);
  rb_define_method(rb_cTexture, "undump",         Texture_undump,          2);
  rb_define_method(rb_cTexture, "width",          Texture_width,           0);

  symbol_add            = ID2SYM(rb_intern("add"));
  symbol_alpha          = ID2SYM(rb_intern("alpha"));
  symbol_angle          = ID2SYM(rb_intern("angle"));
  symbol_blend_type     = ID2SYM(rb_intern("blend_type"));
  symbol_camera_angle   = ID2SYM(rb_intern("camera_angle"));
  symbol_camera_height  = ID2SYM(rb_intern("camera_height"));
  symbol_camera_x       = ID2SYM(rb_intern("camera_x"));
  symbol_camera_y       = ID2SYM(rb_intern("camera_y"));
  symbol_center_x       = ID2SYM(rb_intern("center_x"));
  symbol_center_y       = ID2SYM(rb_intern("center_y"));
  symbol_distance       = ID2SYM(rb_intern("distance"));
  symbol_horizontal     = ID2SYM(rb_intern("horizontal"));
  symbol_intersection_x = ID2SYM(rb_intern("intersection_x"));
  symbol_intersection_y = ID2SYM(rb_intern("intersection_y"));
  symbol_loop           = ID2SYM(rb_intern("loop"));
  symbol_saturation     = ID2SYM(rb_intern("saturation"));
  symbol_scale_x        = ID2SYM(rb_intern("scale_x"));
  symbol_scale_y        = ID2SYM(rb_intern("scale_y"));
  symbol_src_height     = ID2SYM(rb_intern("src_height"));
  symbol_src_width      = ID2SYM(rb_intern("src_width"));
  symbol_src_x          = ID2SYM(rb_intern("src_x"));
  symbol_src_y          = ID2SYM(rb_intern("src_y"));
  symbol_sub            = ID2SYM(rb_intern("sub"));
  symbol_tone_blue      = ID2SYM(rb_intern("tone_blue"));
  symbol_tone_green     = ID2SYM(rb_intern("tone_green"));
  symbol_tone_red       = ID2SYM(rb_intern("tone_red"));
  symbol_vanishing_x    = ID2SYM(rb_intern("vanishing_x"));
  symbol_vanishing_y    = ID2SYM(rb_intern("vanishing_y"));
  symbol_vertical       = ID2SYM(rb_intern("vertical"));
}
