require "mkmf"

if /mswin32/ =~ CONFIG["arch"]
  raise "not supported arch: #{CONFIG["arch"]}"
end

sdl_config = with_config("sdl-config", "sdl-config")
$CFLAGS     += " " + `env #{sdl_config} --cflags`.chomp
$LOCAL_LIBS += " " + `env #{sdl_config} --libs`.chomp

$CFLAGS += " -finline-functions -Wall -std=c99"

have_library("z");
have_library("png");
have_library("SDL_image");

create_makefile("starruby")
