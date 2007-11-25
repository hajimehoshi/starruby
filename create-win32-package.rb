#!/usr/bin/env ruby

require "fileutils"
include FileUtils

def main
  show_usage if ARGV.length != 0
  version = "0.0.0"
  open("win32/readme.txt") do |fp|
    version = fp.gets[/\d+\.\d+\.\d+/]
  end
  main_dir = "starruby-#{version}-win32"
  mkdir_p(main_dir, :verbose => true)
  Dir["win32/*"].each do |path|
    cp(path, main_dir, :verbose => true)
  end
  Dir["examples/**/*\0starruby.so"].each do |path|
    next unless FileTest.file?(path)
    dir = File.join(main_dir, File.dirname(path))
    unless FileTest.directory?(dir)
      mkdir_p(dir, :verbose => true)
    end
    cp(path, dir, :verbose => true)
  end
end

def show_usage
  $stderr.puts("Usage: #{$0}")
  exit(1)
end

main
