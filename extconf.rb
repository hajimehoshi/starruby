require "mkmf"

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

$CFLAGS += " -finline-functions -Wall -std=c99 -mfpmath=sse -msse2 -funit-at-a-time"

have_library("SDL_mixer") or exit(false)
have_library("SDL_ttf") or exit(false)
have_header("png.h") or exit(false)
have_header("zlib.h") or exit(false)

have_header("fontconfig/fontconfig.h") and have_library("fontconfig", "FcInit")

if arg_config("--debug", false)
  $CFLAGS += " -DDEBUG -O0 -g"
else
  $CFLAGS += " -O3"
end

create_makefile("starruby")
