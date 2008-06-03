#include <SDL.h>
#include <SDL_main.h>
#include <ruby.h>
#include "starruby.h"

int main(int argc, char* argv[])
{
  ruby_init();
  ruby_options(argc, argv);
  Init_starruby();
  ruby_run();
  return 0;
}
