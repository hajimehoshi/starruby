require "mkmf"

case CONFIG["arch"]
when /mingw32/
  $CFLAGS += " -DWIN32"
when /mswin32|cygwin|bccwin32|interix|djgpp/
  raise "not supported arch: #{CONFIG["arch"]}"
end

$CFLAGS  += " " + `env sdl-config --cflags`.chomp
$LDFLAGS += " " + `env sdl-config --libs`.chomp

$CFLAGS += " -finline-functions -Wall -std=c99"

$LDFLAGS += " -lSDL_image" # have_library?

if arg_config("--debug", false)
  $CFLAGS += " -DDEBUG -O0"
end

create_makefile("starruby")
