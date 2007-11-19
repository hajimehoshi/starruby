require "mkmf"

case CONFIG["arch"]
when /mingw32/
  $CFLAGS += " -DWIN32"
when /mswin32|cygwin|bccwin32|interix|djgpp/
  raise "not supported arch: #{CONFIG["arch"]}"
end

$CFLAGS  += " " + `env sdl-config --cflags`.chomp
$LDFLAGS += " " + `env sdl-config --libs`.chomp

$CFLAGS += " -finline-functions -Wall -std=c99 -mfpmath=sse -msse2 -funit-at-a-time"

$LDFLAGS += " -lSDL_image" # have_library?
$LDFLAGS += " -lSDL_mixer" # have_library?
$LDFLAGS += " -lSDL_ttf" # have_library?

if have_header("fontconfig/fontconfig.h") and have_library("fontconfig", "FcInit")
  $CFLAGS += " -DUSE_FONTCONFIG"
  $LDFLAGS += " -lfontconfig"
end

if arg_config("--debug", false)
  $CFLAGS += " -DDEBUG -O0"
else
  $CFLAGS += " -O3"
end

create_makefile("starruby")
