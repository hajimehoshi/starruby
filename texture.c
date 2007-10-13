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
  texture->pixels = ALLOC_N(struct Color, texture->width * texture->height);
  MEMZERO(texture->pixels, struct Color, texture->width * texture->height);
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
  texture->pixels = NULL; // TODO
  
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
  
  SDL_Surface* surface = IMG_Load(path);
  if (!surface) {
    rb_raise_sdl_image_error();
    return Qnil;
  }

  VALUE rbTexture = rb_funcall(self, rb_intern("new"), 2,
                               INT2NUM(surface->w), INT2NUM(surface->h));

  struct Texture* texture;
  Data_Get_Struct(rbTexture, struct Texture, texture);
  
  SDL_FreeSurface(surface);
  
  return rbTexture;
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

static VALUE Texture_get_pixel(VALUE self, VALUE rbX, VALUE rbY)
{
  int x = NUM2INT(rbX);
  int y = NUM2INT(rbY);
  
  struct Texture* texture;
  Data_Get_Struct(self, struct Texture, texture);
  
  if (x < 0 || texture->width <= x || y < 0 || texture->height <= y) {
    char errorMessage[256];
    snprintf(errorMessage, 256, "index out of range: (%d, %d)", x, y);
    rb_raise(rb_eIndexError, errorMessage);
    return Qnil;
  }
  
  struct Color color = texture->pixels[x + y * texture->width];
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

static VALUE Texture_set_pixel(VALUE self, VALUE rbX, VALUE rbY, VALUE rbColor)
{
  int x = NUM2INT(rbX);
  int y = NUM2INT(rbY);
  
  struct Texture* texture;
  Data_Get_Struct(self, struct Texture, texture);

  if (x < 0 || texture->width <= x || y < 0 || texture->height <= y) {
    char errorMessage[256];
    snprintf(errorMessage, 256, "index out of range: (%d, %d)", x, y);
    rb_raise(rb_eIndexError, errorMessage);
    return rbColor;;
  }

  struct Color* color;
  Data_Get_Struct(rbColor, struct Color, color);
  
  texture->pixels[x + y * texture->width] = *color;
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
  VALUE rb_cTexture =
    rb_define_class_under(rb_mStarRuby, "Texture", rb_cObject);
  rb_define_alloc_func(rb_cTexture, Texture_alloc);
  rb_define_private_method(rb_cTexture, "initialize", Texture_initialize, 2);
  rb_define_private_method(rb_cTexture, "initialize_copy",
                           Texture_initialize_copy, 1);
  rb_define_singleton_method(rb_cTexture, "load", Texture_load, 1);
  rb_define_method(rb_cTexture, "dispose",   Texture_dispose,   0);
  rb_define_method(rb_cTexture, "disposed?", Texture_disposed,  0);
  rb_define_method(rb_cTexture, "get_pixel", Texture_get_pixel, 2);
  rb_define_method(rb_cTexture, "height",    Texture_height,    0);
  rb_define_method(rb_cTexture, "set_pixel", Texture_set_pixel, 3);
  rb_define_method(rb_cTexture, "size",      Texture_size,      0);
  rb_define_method(rb_cTexture, "width",     Texture_width,     0);
}
