#include <SDL.h>
#include <SDL_main.h>
#include <ruby.h>

int main(int argc, char* argv[])
{
  if (argc <= 1)
    return EXIT_SUCCESS;
  ruby_init();
  rb_eval_string(
    "$LOAD_PATH.clear\n"
    "$LOAD_PATH << File.expand_path('./lib/1.8')\n"
    "$LOAD_PATH << File.expand_path('./lib/1.8/i386-mingw32')\n"
    "$LOAD_PATH << File.expand_path('.')\n"
    "$LOAD_PATH << '.'\n");
  rb_iv_set(rb_eval_string("self"), "@path", rb_str_new2(argv[1]));
  rb_eval_string(
    "dir   = File.dirname(@path)\n"
    "file  = File.basename(@path)\n"
    "@path = nil\n");
  int state;
  rb_eval_string_protect(
    "Dir.chdir(dir) do\n"
    "  load(file)\n"
    "end\n",
    &state);
  if (state)
    rb_eval_string("$stderr.puts($!.to_s)\n"
                   "$stderr.puts($@.to_s)");
  return EXIT_SUCCESS;
}
