#!/usr/bin/env ruby

require "fileutils"
include FileUtils

def main
  show_usage if ARGV.length != 1
  version = ARGV[0]
  main_dir     = "starruby-#{version}-win32"
  examples_dir = "#{main_dir}/examples"
  mkdir_p(main_dir)
  mkdir_p(examples_dir)
  cp(Dir["win32/*\0starruby.so"], main_dir)
  cp(Dir["examples/*"],           examples_dir)
end

def show_usage
  $stderr.puts("Usage: #{$0} version")
  exit(1)
end

main
