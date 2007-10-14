require "mkmf"

if /mswin32/ =~ CONFIG["arch"]
  raise "not supported arch: #{CONFIG["arch"]}"
end

$CFLAGS  += " " + `env sdl-config --cflags`.chomp
$LDFLAGS += " " + `env sdl-config --libs`.chomp

$CFLAGS += " -finline-functions -Wall -std=c99"

$LDFLAGS += " -lSDL_image" # have_library?

create_makefile("starruby")
