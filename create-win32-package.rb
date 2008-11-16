#!/usr/bin/env ruby

require "fileutils"
include FileUtils

def main
  show_usage if ARGV.length != 0
  title = ""
  open("README") do |fp|
    title = fp.gets.chomp
  end
  version = title[/\d+\.\d+\.\d+/]
  main_dir = "starruby-#{version}-win32"
  mkdir_p(main_dir, :verbose => true)
  # readme
  open("win32/readmes/win32.txt", "r") do |fp|
    open(File.join(main_dir, "readme.txt"), "w") do |fp2|
      fp2.write(fp.read.sub("%title%", title))
    end
  end
  # dlls
  Dir.glob("win32/*\0win32/dll/*.dll") do |path|
    next unless FileTest.file?(path)
    dir = File.join(main_dir, File.dirname(path.split("/")[1..-1].join("/")))
    mkdir_p(dir, :verbose => true) unless FileTest.directory?(dir)
    cp(path, dir, :verbose => true)
  end
  # samples
  Dir.glob("samples/**/*") do |path|
    next unless FileTest.file?(path)
    dir = File.join(main_dir, File.dirname(path))
    mkdir_p(dir, :verbose => true) unless FileTest.directory?(dir)
    cp(path, dir, :verbose => true)
  end
  # starruby so
  mkdir(File.join(main_dir, "ext"), :verbose => true)
  cp("starruby.so", File.join(main_dir, "ext"), :verbose => true)
end

def show_usage
  $stderr.puts("Usage: #{$0}")
  exit(1)
end

main
