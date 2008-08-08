require "mkmf"

if arg_config("--gp2x", false)
=begin
  case CONFIG["arch"]
  when /darwin/
    # http://wiki.gp2x.org/wiki/Setting_up_a_development_environment_%28Mac%29
    sdl_base      = "/opt/local/gp2x/bin/arm-open2x-linux-" # todo
    libpng_base   = "" # todo
    cross_compile = "/opt/local/devkitpro/devkitGP2X/bin/arm-linux-"
  when /linux/
    sdl_base      = "/opt/open2x/gcc-4.1.1-glibc-2.3.6/bin/"
    libpng_base   = "/opt/open2x/gcc-4.1.1-glibc-2.3.6/bin/"
    cross_compile = "/opt/open2x/gcc-4.1.1-glibc-2.3.6/bin/arm-open2x-linux-"
  else
    raise "not supported arch: #{CONFIG["arch"]}"
  end
  CONFIG["CC"]    = "#{cross_compile}gcc"
  CONFIG["STRIP"] = "#{cross_compile}strip"

  $CFLAGS  += " " + `#{sdl_base}sdl-config --cflags`.chomp
  $CFLAGS  += " " + `#{libpng_base}libpng-config --cflags`.chomp
  $LDFLAGS += " " + `#{sdl_base}sdl-config --libs`.chomp
  $LDFLAGS += " " + `#{libpng_base}libpng-config --ldflags`.chomp
  $CFLAGS += " -finline-functions -Wall -std=c99-funit-at-a-time"

  $libs += " -lSDL_mixer -lSDL_ttf"
  have_header("png.h") or exit(false)
  have_header("zlib.h") or exit(false)

  create_makefile("starruby")
  exit
=end
  $CFLAGS += " -DGP2X"
end

case CONFIG["arch"]
when /mingw32/
  $CFLAGS += " -DWIN32"
when /mswin32|cygwin|bccwin32|interix|djgpp/
  raise "not supported arch: #{CONFIG["arch"]}"
end

$CFLAGS     += " " + `env libpng-config --cflags`.chomp
$CFLAGS     += " " + `env sdl-config --cflags`.chomp
$LOCAL_LIBS += " " + `env libpng-config --libs`.chomp
$LOCAL_LIBS += " " + `env sdl-config --libs`.chomp

have_header("png.h") or exit(false)
have_header("zlib.h") or exit(false)
have_library("SDL_mixer", "Mix_OpenAudio") or exit(false)
have_library("SDL_ttf", "TTF_Init") or exit(false)

if CONFIG["arch"] =~ /linux|darwin/
  have_header("fontconfig/fontconfig.h") or exit(false)
  have_library("fontconfig", "FcInit") or exit(false)
end

$CFLAGS += " -finline-functions -Wall -W -Wpointer-arith -Wno-unused-parameter -pedantic -std=c99 -funit-at-a-time"
$CFLAGS += " -mfpmath=sse -msse2" if RUBY_PLATFORM !~ /^powerpc/ and CONFIG["arch"] !~ /darwin/
$LDFLAGS += " -Wl,--no-undefined" if CONFIG["arch"] !~ /darwin/

if arg_config("--debug", false)
  $CFLAGS += " -DDEBUG -O0 -g -pg"
end

create_makefile("starruby", "./src")
