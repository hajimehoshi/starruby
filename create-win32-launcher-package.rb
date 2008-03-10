#!/usr/bin/env ruby

require "fileutils"
require "rbconfig"

include FileUtils

def main
  show_usage if ARGV.length != 0
  title = ""
  open("README") do |fp|
    title = fp.gets.chomp
  end
  version = title[/\d+\.\d+\.\d+/]
  main_dir = "starruby-#{version}-win32-launcher"
  mkdir_p(main_dir, :verbose => true)
  # readme
  open("win32/readmes/win32-launcher.txt", "r") do |fp|
    open(File.join(main_dir, "readme.txt"), "w") do |fp2|
      fp2.write(fp.read.sub("%title%", title))
    end
  end
  # dlls
  Dir["win32/dll/*.dll"].each do |path|
    cp(path, main_dir, :verbose => true)
  end
  # samples
  Dir["samples/**/*"].each do |path|
    next unless FileTest.file?(path)
    dir = File.join(main_dir, File.dirname(path))
    mkdir_p(dir, :verbose => true) unless FileTest.directory?(dir)
    cp(path, dir, :verbose => true)
  end
  # star ruby so
  cp("starruby.so", main_dir, :verbose => true)
  # launcher exe
  cd("win32/launcher") do
    system("c:\\ruby\\bin\\mkexy.bat", "starruby-launcher.rb")
    system("c:\\ruby\\bin\\exerb.bat", "starruby-launcher.exy", "-cguirt", "-kUTF8")
  end
  cp("win32/launcher/starruby-launcher.exe", main_dir, :verbose => true)
  path = File.join(Config::CONFIG["prefix"], "share/exerb/*.dll")
  exerb_dll = Dir[path][-1]
  # exerb dll
  cp(exerb_dll, main_dir, :verbose => true)
  # ruby libraries
  lib_dir = File.join(main_dir, "lib")
  mkdir(lib_dir, :verbose => true) unless FileTest.directory?(lib_dir)
  cp_r(RbConfig::CONFIG["rubylibdir"], lib_dir, :verbose => true)
  # game.rb
  cp("win32/launcher/game.rb", main_dir)
end

def show_usage
  $stderr.puts("Usage: #{$0}")
  exit(1)
end

main
