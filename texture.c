#include "starruby.h"
#include <png.h>

#define ALPHA(src, dst, a) DIV255((dst << 8) - dst + (src - dst) * a)

volatile static VALUE symbol_add;
volatile static VALUE symbol_alpha;
volatile static VALUE symbol_angle;
volatile static VALUE symbol_blend_type;
volatile static VALUE symbol_camera_height;
volatile static VALUE symbol_camera_pitch;
volatile static VALUE symbol_camera_roll;
volatile static VALUE symbol_camera_x;
volatile static VALUE symbol_camera_y;
volatile static VALUE symbol_camera_yaw;
volatile static VALUE symbol_center_x;
volatile static VALUE symbol_center_y;
volatile static VALUE symbol_distance;
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

typedef enum {
  ALPHA,
  ADD,
  SUB,
} BlendType;

typedef struct {
  double a, b, c, d, tx, ty;
} AffineMatrix;

typedef struct {
  int x, y, z;
} Point;

typedef struct {
  double x, y, z;
} PointF;

typedef Point Vector;
typedef PointF VectorF;

typedef struct {
  int cameraX;
  int cameraY;
  double cameraHeight;
  double cameraYaw;
  double cameraPitch;
  double cameraRoll;
  double distance;
  int intersectionX;
  int intersectionY;
  bool isLoop;
} PerspectiveOptions;

inline static void
CheckDisposed(Texture* texture)
{
  if (!texture->pixels)
    rb_raise(rb_eRuntimeError,
             "can't modify disposed StarRuby::Texture");
}

static VALUE
Texture_load(VALUE self, VALUE rbPath)
{
  volatile VALUE rbCompletePath = GetCompletePath(rbPath, true);
  char* path = StringValuePtr(rbCompletePath);
  FILE* fp = fopen(path, "rb");
  png_byte header[8];
  fread(&header, 1, 8, fp);
  if (png_sig_cmp(header, 0, 8))
    rb_raise(rb_eStarRubyError, "invalid PNG file: %s", path);
  png_structp pngPtr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
                                              NULL, NULL, NULL);
  if (!pngPtr) {
    fclose(fp);
    rb_raise(rb_eStarRubyError, "PNG error: %s", path);
  }
  png_infop infoPtr = png_create_info_struct(pngPtr);
  if (!infoPtr) {
    png_destroy_read_struct(&pngPtr, NULL, NULL);
    fclose(fp);
    rb_raise(rb_eStarRubyError, "PNG error: %s", path);
  }
  png_infop endInfo = png_create_info_struct(pngPtr);
  if (!endInfo) {
    png_destroy_read_struct(&pngPtr, &infoPtr, NULL);
    fclose(fp);
    rb_raise(rb_eStarRubyError, "PNG error: %s", path);
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
  if (width <= 0)
    rb_raise(rb_eArgError, "width less than or equal to 0");
  if (height <= 0)
    rb_raise(rb_eArgError, "height less than or equal to 0");
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
  if (!NIL_P(val = rb_hash_aref(rbOptions, symbol_camera_yaw)))
    options->cameraYaw = NUM2DBL(val);
  if (!NIL_P(val = rb_hash_aref(rbOptions, symbol_camera_pitch)))
    options->cameraPitch = NUM2DBL(val);
  if (!NIL_P(val = rb_hash_aref(rbOptions, symbol_camera_roll)))
    options->cameraRoll = NUM2DBL(val);
  if (!NIL_P(val = rb_hash_aref(rbOptions, symbol_distance)))
    options->distance = NUM2DBL(val);
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
  PerspectiveOptions options;
  AssignPerspectiveOptions(&options, rbOptions);
  double cosYaw   = cos(options.cameraYaw);
  double sinYaw   = sin(options.cameraYaw);
  double cosPitch = cos(options.cameraPitch);
  double sinPitch = sin(options.cameraPitch);
  double cosRoll  = cos(options.cameraRoll);
  double sinRoll  = sin(options.cameraRoll);
  double x = NUM2INT(rbX) - options.cameraX;
  double y = NUM2DBL(rbHeight);
  double z = NUM2INT(rbY) - options.cameraY;
  double x2, y2, z2;
  x2 = cosYaw * x  + sinYaw * z;
  z2 = -sinYaw * x + cosYaw * z;
  x = x2;
  z = z2;
  y2 = sinPitch * z + cosPitch * (y - options.cameraHeight)
    + options.cameraHeight;
  z2 = cosPitch * z - sinPitch * (y - options.cameraHeight);
  y = y2;
  z = z2;
  volatile VALUE rbResult = rb_ary_new3(3, Qnil, Qnil, Qnil);
  OBJ_FREEZE(rbResult);
  if (z == 0)
    return rbResult;
  double scale = -options.distance / z;
  double screenX = x * scale;
  double screenY = (options.cameraHeight - y) * scale;
  double screenX2 = cosRoll  * screenX + sinRoll * screenY;
  double screenY2 = -sinRoll * screenX + cosRoll * screenY;
  screenX = screenX2 + options.intersectionX;
  screenY = screenY2 + options.intersectionY;
  long screenXLong = (long)screenX;
  long screenYLong = (long)screenY;
  RARRAY_PTR(rbResult)[0] = FIXABLE(screenXLong) ? LONG2FIX(screenXLong) : Qnil; 
  RARRAY_PTR(rbResult)[1] = FIXABLE(screenYLong) ? LONG2FIX(screenYLong) : Qnil;
  RARRAY_PTR(rbResult)[2] = rb_float_new(scale);
  return rbResult;
}

static VALUE Texture_change_hue_bang(VALUE, VALUE);
static VALUE
Texture_change_hue(VALUE self, VALUE rbAngle)
{
  Texture* texture;
  Data_Get_Struct(self, Texture, texture);
  CheckDisposed(texture);
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
  CheckDisposed(texture);
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
      case 0: r = v255;  g = cc255; b = aa255; break;
      case 1: r = bb255; g = v255;  b = aa255; break;
      case 2: r = aa255; g = v255;  b = cc255; break;
      case 3: r = aa255; g = bb255; b = v255;  break;
      case 4: r = cc255; g = aa255; b = v255;  break;
      case 5: r = v255;  g = aa255; b = bb255; break;
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
  CheckDisposed(texture);
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
  CheckDisposed(texture);
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
  CheckDisposed(texture);
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
  CheckDisposed(texture);
  int rectX = NUM2INT(rbX);
  int rectY = NUM2INT(rbY);
  int rectWidth  = NUM2INT(rbWidth);
  int rectHeight = NUM2INT(rbHeight);
  if (rectX < 0) {
    rectWidth -= -rectX;
    rectX = 0;
  }
  if (rectY < 0) {
    rectHeight -= -rectY;
    rectY = 0;
  }
  if (texture->width <= rectX || texture->height <= rectY)
    return Qnil;
  if (texture->width <= rectX + rectWidth)
    rectWidth = texture->width - rectX;
  if (texture->height <= rectY + rectHeight)
    rectHeight = texture->height - rectY;
  if (rectWidth <= 0 || rectHeight <= 0)
    return Qnil;
  Color* color;
  Data_Get_Struct(rbColor, Color, color);
  Pixel* pixels = &(texture->pixels[rectX + rectY * texture->width]);
  int paddingJ = texture->width - rectWidth;
  for (int j = rectY; j < rectY + rectHeight; j++, pixels += paddingJ)
    for (int i = rectX; i < rectX + rectWidth; i++, pixels++)
      pixels->color = *color;
  return Qnil;
}

static VALUE
Texture_get_pixel(VALUE self, VALUE rbX, VALUE rbY)
{
  Texture* texture;
  Data_Get_Struct(self, Texture, texture);
  CheckDisposed(texture);
  int x = NUM2INT(rbX);
  int y = NUM2INT(rbY);
  if (x < 0 || texture->width <= x || y < 0 || texture->height <= y)
    rb_raise(rb_eArgError, "index out of range: (%d, %d)", x, y);
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
  CheckDisposed(texture);
  return INT2NUM(texture->height);
}

#define RENDER_PIXEL(_dst, _srcP)                                       \
  do {                                                                  \
    if (_dst.alpha == 0) {                                              \
      _dst = *_srcP;                                                    \
    } else {                                                            \
      if (_dst.alpha < _srcP->alpha)                                    \
        _dst.alpha = _srcP->alpha;                                      \
      _dst.red   = ALPHA(_srcP->red,   _dst.red,   _srcP->alpha);       \
      _dst.green = ALPHA(_srcP->green, _dst.green, _srcP->alpha);       \
      _dst.blue  = ALPHA(_srcP->blue,  _dst.blue,  _srcP->alpha);       \
    }                                                                   \
  } while (false)

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
  CheckDisposed(srcTexture);
  Texture* dstTexture;
  Data_Get_Struct(self, Texture, dstTexture);
  CheckDisposed(dstTexture);
  if (srcTexture == dstTexture)
    rb_raise(rb_eRuntimeError, "can't render self in perspective");
  PerspectiveOptions options;
  AssignPerspectiveOptions(&options, rbOptions);
  if (options.cameraHeight == 0)
    return Qnil;
  int srcWidth  = srcTexture->width;
  int srcHeight = srcTexture->height;
  int dstWidth  = dstTexture->width;
  int dstHeight = dstTexture->height;
  double cosYaw   = cos(options.cameraYaw);
  double sinYaw   = sin(options.cameraYaw);
  double cosPitch = cos(options.cameraPitch);
  double sinPitch = sin(options.cameraPitch);
  double cosRoll  = cos(options.cameraRoll);
  double sinRoll  = sin(options.cameraRoll);
  VectorF screenDX = {
    cosRoll * cosYaw + sinRoll * sinPitch * sinYaw,
    sinRoll * -cosPitch,
    cosRoll * sinYaw - sinRoll * sinPitch * cosYaw,
  };
  VectorF screenDY = {
    -sinRoll * cosYaw + cosRoll * sinPitch * sinYaw,
    cosRoll * -cosPitch,
    -sinRoll * sinYaw - cosRoll * sinPitch * cosYaw,
  };
  PointF intersection = {
    options.distance * (cosPitch * sinYaw),
    options.distance * sinPitch + options.cameraHeight,
    options.distance * (-cosPitch * cosYaw),
  };
  PointF screenO;
  screenO.x = intersection.x
    - options.intersectionX * screenDX.x
    - options.intersectionY * screenDY.x;
  screenO.y = intersection.y
    - options.intersectionX * screenDX.y
    - options.intersectionY * screenDY.y;
  screenO.z = intersection.z
    - options.intersectionX * screenDX.z
    - options.intersectionY * screenDY.z;
  int cameraHeight = options.cameraHeight;
  Pixel* src = srcTexture->pixels;
  Pixel* dst = dstTexture->pixels;
  PointF screenP;
  for (int j = 0; j < dstHeight; j++) {
    screenP.x = screenO.x + j * screenDY.x;
    screenP.y = screenO.y + j * screenDY.y;
    screenP.z = screenO.z + j * screenDY.z;
    for (int i = 0; i < dstWidth; i++, dst++,
           screenP.x += screenDX.x,
           screenP.y += screenDX.y,
           screenP.z += screenDX.z) {
      if (cameraHeight != screenP.y &&
          ((0 < cameraHeight && screenP.y < cameraHeight) ||
           (cameraHeight < 0 && cameraHeight < screenP.y))) {
        double scale = cameraHeight / (cameraHeight - screenP.y);
        int srcX = (int)((screenP.x) * scale + options.cameraX);
        int srcZ = (int)((screenP.z) * scale + options.cameraY);
        if (options.isLoop) {
          srcX %= srcWidth;
          if (srcX < 0)
            srcX += srcWidth;
          srcZ %= srcHeight;
          if (srcZ < 0)
            srcZ += srcHeight;
        }
        if (options.isLoop ||
            (0 <= srcX && srcX < srcWidth && 0 <= srcZ && srcZ < srcHeight)) {
          Color* srcColor = &(src[srcX + srcZ * srcWidth].color);
          RENDER_PIXEL(dst->color, srcColor);
        }
      }
    }
  }
  return Qnil;
}

static VALUE
Texture_render_line(VALUE self,
                    VALUE rbX1, VALUE rbY1, VALUE rbX2, VALUE rbY2, VALUE rbColor)
{
  rb_check_frozen(self);
  int x1 = NUM2INT(rbX1);
  int y1 = NUM2INT(rbY1);
  int x2 = NUM2INT(rbX2);
  int y2 = NUM2INT(rbY2);
  Texture* texture;
  Data_Get_Struct(self, Texture, texture);
  CheckDisposed(texture);
  Color* color;
  Data_Get_Struct(rbColor, Color, color);
  int x = x1;
  int y = y1;
  int dx = abs(x2 - x1);
  int dy = abs(y2 - y1);
  int signX = (x1 <= x2) ? 1 : -1;
  int signY = (y1 <= y2) ? 1 : -1;
  if (dy <= dx) {
    int e = dx;
    int eLimit = dx * 2;
    for (int i = 0; i <= dx; i++) {
      if (0 <= x && x < texture->width && 0 <= y && y < texture->height) {
        Pixel* pixel = &(texture->pixels[x + y * texture->width]);
        RENDER_PIXEL(pixel->color, color);
      }
      x += signX;
      e += 2 * dy;
      if (eLimit <= e) {
        e -= eLimit;
        y += signY;
      }
    }
  } else {
    int e = dy;
    int eLimit = dy * 2;
    for (int i = 0; i <= dy; i++) {
      if (0 <= x && x < texture->width && 0 <= y && y < texture->height) {
        Pixel* pixel = &(texture->pixels[x + y * texture->width]);
        RENDER_PIXEL(pixel->color, color);
      }
      y += signY;
      e += 2 * dx;
      if (eLimit <= e) {
        e -= eLimit;
        x += signX;
      }
    }
  }
  return Qnil;
}

static VALUE
Texture_render_pixel(VALUE self, VALUE rbX, VALUE rbY, VALUE rbColor)
{
  rb_check_frozen(self);
  Texture* texture;
  Data_Get_Struct(self, Texture, texture);
  CheckDisposed(texture);
  int x = NUM2INT(rbX);
  int y = NUM2INT(rbY);
  if (x < 0 || texture->width <= x || y < 0 || texture->height <= y)
    return Qnil;
  Color* color;
  Data_Get_Struct(rbColor, Color, color);
  Pixel* pixel = &(texture->pixels[x + y * texture->width]);
  RENDER_PIXEL(pixel->color, color);
  return Qnil;
}

static VALUE
Texture_render_rect(VALUE self, VALUE rbX, VALUE rbY,
                    VALUE rbWidth, VALUE rbHeight, VALUE rbColor)
{
  rb_check_frozen(self);
  Texture* texture;
  Data_Get_Struct(self, Texture, texture);
  CheckDisposed(texture);
  int rectX = NUM2INT(rbX);
  int rectY = NUM2INT(rbY);
  int rectWidth  = NUM2INT(rbWidth);
  int rectHeight = NUM2INT(rbHeight);
  if (rectX < 0) {
    rectWidth -= -rectX;
    rectX = 0;
  }
  if (rectY < 0) {
    rectHeight -= -rectY;
    rectY = 0;
  }
  if (texture->width <= rectX || texture->height <= rectY)
    return Qnil;
  if (texture->width <= rectX + rectWidth)
    rectWidth = texture->width - rectX;
  if (texture->height <= rectY + rectHeight)
    rectHeight = texture->height - rectY;
  if (rectWidth <= 0 || rectHeight <= 0)
    return Qnil;
  Color* color;
  Data_Get_Struct(rbColor, Color, color);
  Pixel* pixels = &(texture->pixels[rectX + rectY * texture->width]);
  int paddingJ = texture->width - rectWidth;
  for (int j = rectY; j < rectY + rectHeight; j++, pixels += paddingJ)
    for (int i = rectX; i < rectX + rectWidth; i++, pixels++)
      RENDER_PIXEL(pixels->color, color);
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
  if (!RSTRING_LEN(rbText))
    rb_raise(rb_eArgError, "empty text");
  char* text = StringValuePtr(rbText);
  Font* font;
  Data_Get_Struct(rbFont, Font, font);
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
  if (!textSurfaceRaw)
    rb_raise_sdl_ttf_error();
  SDL_Surface* textSurface = SDL_ConvertSurface(textSurfaceRaw, &(SDL_PixelFormat) {
      .palette = NULL, .BitsPerPixel = 32, .BytesPerPixel = 4,
        .Rmask = 0x00ff0000, .Gmask = 0x0000ff00,
        .Bmask = 0x000000ff, .Amask = 0xff000000,
        .colorkey = 0, .alpha = 255,
        }, SDL_SWSURFACE);
  SDL_FreeSurface(textSurfaceRaw);
  textSurfaceRaw = NULL;
  if (!textSurface)
    rb_raise_sdl_error();
  SDL_LockSurface(textSurface);
  Pixel* src = (Pixel*)(textSurface->pixels);
  Pixel* dst = textTexture->pixels;
  int size = textTexture->width * textTexture->height;
  for (int i = 0; i < size; i++, src++, dst++) {
    if (src->value) {
      dst->color = *color;
      if (color->alpha == 255)
        dst->color.alpha = src->color.red;
      else
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

static VALUE
Texture_render_texture(int argc, VALUE* argv, VALUE self)
{
  rb_check_frozen(self);
  Texture* dstTexture;
  Data_Get_Struct(self, Texture, dstTexture);
  CheckDisposed(dstTexture);

  volatile VALUE rbTexture, rbX, rbY, rbOptions;
  rb_scan_args(argc, argv, "31", &rbTexture, &rbX, &rbY, &rbOptions);
  if (NIL_P(rbOptions))
    rbOptions = rb_hash_new();

  Texture* srcTexture;
  Data_Get_Struct(rbTexture, Texture, srcTexture);
  CheckDisposed(srcTexture);

  int srcTextureWidth  = srcTexture->width;
  int srcTextureHeight = srcTexture->height;
  int dstTextureWidth  = dstTexture->width;
  int dstTextureHeight = dstTexture->height;

  int srcX = 0;
  int srcY = 0;
  int srcWidth = 0;
  int srcHeight = 0;
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

  if (!(0 <= srcX && srcX < srcTextureWidth))
    rb_raise(rb_eArgError, "invalid src_x: %d", srcX);
  if (!(0 <= srcY && srcY < srcTextureHeight))
    rb_raise(rb_eArgError, "invalid src_y: %d", srcY);
  if (!(0 <= srcWidth && srcWidth <= srcTextureWidth - srcX))
    rb_raise(rb_eArgError, "invalid src_width: %d", srcWidth);
  if (!(0 <= srcHeight && srcHeight <= srcTextureHeight - srcY))
    rb_raise(rb_eArgError, "invalid src_height: %d", srcHeight);

  if (srcTexture != dstTexture &&
      scaleX == 1 && scaleY == 1 && angle == 0 &&
      saturation == 255 && toneRed == 0 && toneGreen == 0 && toneBlue == 0 &&
      blendType == ALPHA) {
    int dstX = (int)NUM2DBL(rbX);
    int dstY = (int)NUM2DBL(rbY);
    if (dstX < 0) {
      srcX -= dstX;
      srcWidth += dstX;
      if (srcTextureWidth <= srcX || srcWidth < 0)
        return Qnil;
      dstX = 0;
    }
    if (dstY < 0) {
      srcY -= dstY;
      srcHeight += dstY;
      if (srcTextureHeight <= srcY || srcHeight < 0)
        return Qnil;
      dstY = 0;
    }
    int width  = MIN(srcWidth,  dstTextureWidth - dstX);
    int height = MIN(srcHeight, dstTextureHeight - dstY);
    Pixel* src = &(srcTexture->pixels[srcX + srcY * srcTextureWidth]);
    Pixel* dst = &(dstTexture->pixels[dstX + dstY * dstTextureWidth]);
    int srcPadding = srcTextureWidth - width;
    int dstPadding = dstTextureWidth - width;
    for (int j = 0; j < height; j++, src += srcPadding, dst += dstPadding) {
      for (int i = 0; i < width; i++, src++, dst++) {
        if (dst->color.alpha == 0) {
          dst->color.alpha = DIV255(src->color.alpha * alpha);
          dst->color.red   = src->color.red;
          dst->color.green = src->color.green;
          dst->color.blue  = src->color.blue;
        } else {
          uint8_t pixelAlpha = DIV255(src->color.alpha * alpha);
          if (dst->color.alpha < pixelAlpha)
            dst->color.alpha = pixelAlpha;
          dst->color.red   = ALPHA(src->color.red,   dst->color.red,   pixelAlpha);
          dst->color.green = ALPHA(src->color.green, dst->color.green, pixelAlpha);
          dst->color.blue  = ALPHA(src->color.blue,  dst->color.blue,  pixelAlpha);
        }
      }
    }
    return Qnil;
  }

  AffineMatrix mat = {
    .a = 1, .c = 0, .tx = 0,
    .b = 0, .d = 1, .ty = 0,
  };
  if (scaleX != 1 || scaleY != 1 || angle != 0) {
    mat.tx -= centerX;
    mat.ty -= centerY;
    if (scaleX != 1) {
      mat.a  *= scaleX;
      mat.b  *= scaleX;
      mat.tx *= scaleX;
    }
    if (scaleY != 1) {
      mat.c  *= scaleY;
      mat.d  *= scaleY;
      mat.ty *= scaleY;
    }
    if (angle != 0) {
      double a  = mat.a;
      double b  = mat.b;
      double c  = mat.c;
      double d  = mat.d;
      double tx = mat.tx;
      double ty = mat.ty;
      double cosAngle = cos(angle);
      double sinAngle = sin(angle);
      mat.a  = cosAngle * a  - sinAngle * c;
      mat.b  = cosAngle * b  - sinAngle * d;
      mat.c  = sinAngle * a  + cosAngle * c;
      mat.d  = sinAngle * b  + cosAngle * d;
      mat.tx = cosAngle * tx - sinAngle * ty;
      mat.ty = sinAngle * tx + cosAngle * ty;
    }
    mat.tx += centerX;
    mat.ty += centerY;
  }
  mat.tx += NUM2DBL(rbX);
  mat.ty += NUM2DBL(rbY);
  double det = mat.a * mat.d - mat.b * mat.c;
  if (det == 0)
    return Qnil;

  double dstX00 = mat.tx;
  double dstY00 = mat.ty;
  double dstX01 = mat.b * srcHeight + mat.tx;
  double dstY01 = mat.d * srcHeight + mat.ty;
  double dstX10 = mat.a * srcWidth  + mat.tx;
  double dstY10 = mat.c * srcWidth  + mat.ty;
  double dstX11 = mat.a * srcWidth  + mat.b * srcHeight + mat.tx;
  double dstY11 = mat.c * srcWidth  + mat.d * srcHeight + mat.ty;
  double dstX0 = MIN(MIN(MIN(dstX00, dstX01), dstX10), dstX11);
  double dstY0 = MIN(MIN(MIN(dstY00, dstY01), dstY10), dstY11);
  double dstX1 = MAX(MAX(MAX(dstX00, dstX01), dstX10), dstX11);
  double dstY1 = MAX(MAX(MAX(dstY00, dstY01), dstY10), dstY11);
  if (dstTextureWidth <= dstX0 || dstTextureHeight <= dstY0 ||
      dstX1 < 0 || dstY1 < 0)
    return Qnil;

  AffineMatrix matInv = {
    .a = mat.d  / det,
    .b = -mat.b / det,
    .c = -mat.c / det,
    .d = mat.a  / det,
  };
  matInv.tx = -(matInv.a * mat.tx + matInv.b * mat.ty);
  matInv.ty = -(matInv.c * mat.tx + matInv.d * mat.ty);
  double srcOX = matInv.a * (dstX0 + 0.5) + matInv.b * (dstY0 + 0.5)
    + matInv.tx + srcX;
  double srcOY = matInv.c * (dstX0 + 0.5) + matInv.d * (dstY0 + 0.5)
    + matInv.ty + srcY;
  double srcDXX = matInv.a;
  double srcDXY = matInv.c;
  double srcDYX = matInv.b;
  double srcDYY = matInv.d;

  if (dstX0 < 0) {
    srcOX -= dstX0 * srcDXX;
    srcOY -= dstX0 * srcDXY;
    dstX0 = 0;
  }
  if (dstY0 < 0) {
    srcOX -= dstY0 * srcDYX;
    srcOY -= dstY0 * srcDYY;
    dstY0 = 0;
  }
  int dstX0Int = (int)dstX0;
  int dstY0Int = (int)dstY0;
  int dstWidth  = MIN(dstTextureWidth,  (int)dstX1) - dstX0Int;
  int dstHeight = MIN(dstTextureHeight, (int)dstY1) - dstY0Int;

  int_fast32_t srcOX16  = (int_fast32_t)(srcOX  * (1 << 16));
  int_fast32_t srcOY16  = (int_fast32_t)(srcOY  * (1 << 16));
  int_fast32_t srcDXX16 = (int_fast32_t)(srcDXX * (1 << 16));
  int_fast32_t srcDXY16 = (int_fast32_t)(srcDXY * (1 << 16));
  int_fast32_t srcDYX16 = (int_fast32_t)(srcDYX * (1 << 16));
  int_fast32_t srcDYY16 = (int_fast32_t)(srcDYY * (1 << 16));

  volatile VALUE rbClonedTexture = Qnil;
  if (self == rbTexture) {
    rbClonedTexture = rb_funcall(rbTexture, rb_intern("clone"), 0);
    Data_Get_Struct(rbClonedTexture, Texture, srcTexture);
  }

  int srcX2 = srcX + srcWidth;
  int srcY2 = srcY + srcHeight;
  for (int j = 0; j < dstHeight; j++) {
    int_fast32_t srcI16 = srcOX16 + j * srcDYX16;
    int_fast32_t srcJ16 = srcOY16 + j * srcDYY16;
    Pixel* dst =
      &(dstTexture->pixels[dstX0Int + (dstY0Int + j) * dstTextureWidth]);
    for (int i = 0; i < dstWidth;
         i++, dst++, srcI16 += srcDXX16, srcJ16 += srcDXY16) {
      int_fast32_t srcI = srcI16 >> 16;
      int_fast32_t srcJ = srcJ16 >> 16;
      if (srcX <= srcI && srcI < srcX2 && srcY <= srcJ && srcJ < srcY2) {
        Color srcColor = srcTexture->pixels[srcI + srcJ * srcTextureWidth].color;
        if (saturation < 255) {
          // http://www.poynton.com/ColorFAQ.html
          uint8_t y = (6969 * srcColor.red +
                       23434 * srcColor.green +
                       2365 * srcColor.blue) / 32768;
          srcColor.red   = ALPHA(srcColor.red,   y, saturation);
          srcColor.green = ALPHA(srcColor.green, y, saturation);
          srcColor.blue  = ALPHA(srcColor.blue,  y, saturation);
        }
        if (toneRed) {
          if (0 < toneRed)
            srcColor.red   = ALPHA(255, srcColor.red, toneRed);
          else
            srcColor.red = ALPHA(0,   srcColor.red, -toneRed);
        }
        if (toneGreen) {
          if (0 < toneGreen)
            srcColor.green = ALPHA(255, srcColor.green, toneGreen);
          else
            srcColor.green = ALPHA(0,   srcColor.green, -toneGreen);
        }
        if (toneBlue) {
          if (0 < toneBlue)
            srcColor.blue = ALPHA(255, srcColor.blue, toneBlue);
          else
            srcColor.blue = ALPHA(0, srcColor.blue, -toneBlue);
        }
        if (dst->color.alpha == 0) {
          dst->color.alpha = DIV255(srcColor.alpha * alpha);
          switch (blendType) {
          case ALPHA:
            dst->color.red   = srcColor.red;
            dst->color.green = srcColor.green;
            dst->color.blue  = srcColor.blue;
            break;
          case ADD:
            dst->color.red   = MIN(255, dst->color.red   + srcColor.red);
            dst->color.green = MIN(255, dst->color.green + srcColor.green);
            dst->color.blue  = MIN(255, dst->color.blue  + srcColor.blue);
            break;
          case SUB:
            dst->color.red   = MAX(0, -srcColor.red   + dst->color.red);
            dst->color.green = MAX(0, -srcColor.green + dst->color.green);
            dst->color.blue  = MAX(0, -srcColor.blue  + dst->color.blue);
            break;
          }
        } else {
          uint8_t pixelAlpha = DIV255(srcColor.alpha * alpha);
          if (dst->color.alpha < pixelAlpha)
            dst->color.alpha = pixelAlpha;
          switch (blendType) {
          case ALPHA:
            dst->color.red   = ALPHA(srcColor.red,   dst->color.red,   pixelAlpha);
            dst->color.green = ALPHA(srcColor.green, dst->color.green, pixelAlpha);
            dst->color.blue  = ALPHA(srcColor.blue,  dst->color.blue,  pixelAlpha);
            break;
          case ADD:
            dst->color.red   = MIN(255, dst->color.red   + DIV255(srcColor.red * pixelAlpha));
            dst->color.green = MIN(255, dst->color.green + DIV255(srcColor.green * pixelAlpha));
            dst->color.blue  = MIN(255, dst->color.blue  + DIV255(srcColor.blue * pixelAlpha));
            break;
          case SUB:
            dst->color.red   = MAX(0, -DIV255(srcColor.red * pixelAlpha)   + dst->color.red);
            dst->color.green = MAX(0, -DIV255(srcColor.green * pixelAlpha) + dst->color.green);
            dst->color.blue  = MAX(0, -DIV255(srcColor.blue * pixelAlpha)  + dst->color.blue);
            break;
          }
        }
      } else if ((srcI < srcX && srcDXX <= 0) || (srcX2 <= srcI && 0 <= srcDXX) ||
                 (srcJ < srcY && srcDXY <= 0) || (srcY2 <= srcJ && 0 <= srcDXY)) {
        break;
      }
    }
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
  CheckDisposed(texture);
  volatile VALUE rbPath, rbAlpha;
  rb_scan_args(argc, argv, "11", &rbPath, &rbAlpha);
  if (argc == 1)
    rbAlpha = Qtrue;
  char* path = StringValuePtr(rbPath);
  FILE* fp = fopen(path, "wb");
  if (!fp)
    rb_raise(rb_path2class("Errno::ENOENT"), "%s", path);
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
  CheckDisposed(texture);
  int x = NUM2INT(rbX);
  int y = NUM2INT(rbY);
  if (x < 0 || texture->width <= x || y < 0 || texture->height <= y)
    return Qnil;
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
  CheckDisposed(texture);
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
  CheckDisposed(texture);
  char* format = StringValuePtr(rbFormat);
  int formatLength = RSTRING_LEN(rbFormat);
  int textureSize = texture->width * texture->height;
  Check_Type(rbData, T_STRING);
  if (textureSize * formatLength != RSTRING_LEN(rbData))
    rb_raise(rb_eArgError, "invalid data size: %d expected but was %ld",
             textureSize * formatLength, RSTRING_LEN(rbData));
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
  CheckDisposed(texture);
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
  rb_define_method(rb_cTexture, "dump",           Texture_dump,            1);
  rb_define_method(rb_cTexture, "fill",           Texture_fill,            1);
  rb_define_method(rb_cTexture, "fill_rect",      Texture_fill_rect,       5);
  rb_define_method(rb_cTexture, "get_pixel",      Texture_get_pixel,       2);
  rb_define_method(rb_cTexture, "height",         Texture_height,          0);
  rb_define_method(rb_cTexture, "render_in_perspective",
                   Texture_render_in_perspective, -1);
  rb_define_method(rb_cTexture, "render_line",    Texture_render_line,     5);
  rb_define_method(rb_cTexture, "render_pixel",   Texture_render_pixel,    3);
  rb_define_method(rb_cTexture, "render_rect",    Texture_render_rect,     5);
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
  symbol_camera_height  = ID2SYM(rb_intern("camera_height"));
  symbol_camera_pitch   = ID2SYM(rb_intern("camera_pitch"));
  symbol_camera_roll    = ID2SYM(rb_intern("camera_roll"));
  symbol_camera_x       = ID2SYM(rb_intern("camera_x"));
  symbol_camera_y       = ID2SYM(rb_intern("camera_y"));
  symbol_camera_yaw     = ID2SYM(rb_intern("camera_yaw"));
  symbol_center_x       = ID2SYM(rb_intern("center_x"));
  symbol_center_y       = ID2SYM(rb_intern("center_y"));
  symbol_distance       = ID2SYM(rb_intern("distance"));
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
}
