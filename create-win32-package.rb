#!/usr/bin/env ruby

require "fileutils"
include FileUtils

def main
  show_usage if ARGV.length != 0
  version = "0.0.0"
  open("win32/readme.txt") do |fp|
    version = fp.gets[/\d+\.\d+\.\d+/]
  end
  main_dir     = "starruby-#{version}-win32"
  examples_dir = "#{main_dir}/examples"
  mkdir_p(main_dir)
  mkdir_p(examples_dir)
  cp(Dir["win32/**/*\0starruby.so"], main_dir)
  cp(Dir["examples/**/*"],           examples_dir)
end

def show_usage
  $stderr.puts("Usage: #{$0}")
  exit(1)
end

main
