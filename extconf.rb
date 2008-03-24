require "mkmf"

lib_base     = ""
include_base = ""
sdl_base     = ""

if arg_config("--gp2x", false)
  case CONFIG["arch"]
  when /darwin/
    # http://wiki.gp2x.org/wiki/Setting_up_a_development_environment_%28Mac%29
    sdl_base      = "/opt/local/gp2x/bin/arm-open2x-linux-"
    cross_compile = "/opt/local/devkitpro/devkitGP2X/bin/arm-linux-"
    CONFIG["CC"]    = "#{cross_compile}gcc"
    CONFIG["STRIP"] = "#{cross_compile}strip"

    $CFLAGS  += " " + `#{sdl_base}sdl-config --cflags`.chomp
    # $CFLAGS  += " " + `libpng-config --cflags`.chomp
    $LDFLAGS += " " + `#{sdl_base}sdl-config --libs`.chomp
    # $LDFLAGS += " " + `libpng-config --libs`.chomp
    $CFLAGS += " -finline-functions -Wall -std=c99 -funit-at-a-time"

    $libs += " -lSDL_mixer -lSDL_ttf"
    have_header("png.h") or exit(false)
    have_header("zlib.h") or exit(false)

    create_makefile("starruby")
    exit
  else
    raise "not supported arch: #{CONFIG["arch"]}"
  end
end

case CONFIG["arch"]
when /mingw32/
  $CFLAGS += " -DWIN32"
when /mswin32|cygwin|bccwin32|interix|djgpp/
  raise "not supported arch: #{CONFIG["arch"]}"
end

$CFLAGS  += " " + `env sdl-config --cflags`.chomp
$CFLAGS  += " " + `env libpng-config --cflags`.chomp
$LDFLAGS += " " + `env sdl-config --libs`.chomp
$LDFLAGS += " " + `env libpng-config --libs`.chomp

$CFLAGS += " -finline-functions -Wall -std=c99 -funit-at-a-time"
$CFLAGS += " -mfpmath=sse -msse2" if RUBY_PLATFORM !~ /^powerpc/

have_library("SDL_mixer") or exit(false)
have_library("SDL_ttf") or exit(false)
have_header("png.h") or exit(false)
have_header("zlib.h") or exit(false)

if CONFIG["arch"] =~ /linux|darwin/
  have_header("fontconfig/fontconfig.h") or exit(false)
  have_library("fontconfig", "FcInit") or exit(false)
end

if arg_config("--debug", false)
  $CFLAGS += " -DDEBUG -O0 -g"
else
  $CFLAGS += " -O3"
end

create_makefile("starruby")
