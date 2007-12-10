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
  open("win32/readme.txt.tmpl", "r") do |fp|
    open(File.join(main_dir, "readme.txt"), "w") do |fp2|
      fp2.write(fp.read.sub("%title%", title))
    end
  end
  Dir["win32/**/*"].each do |path|
    next if path =~ /tmpl$/
    next unless FileTest.file?(path)
    dir = File.join(main_dir, File.dirname(path[6..-1]))
    unless FileTest.directory?(dir)
      mkdir_p(dir, :verbose => true)
    end
    cp(path, dir, :verbose => true)
  end
  Dir["samples/**/*"].each do |path|
    next unless FileTest.file?(path)
    dir = File.join(main_dir, File.dirname(path))
    unless FileTest.directory?(dir)
      mkdir_p(dir, :verbose => true)
    end
    cp(path, dir, :verbose => true)
  end
  mkdir(File.join(main_dir, "ext"), :verbose => true)
  cp("starruby.so", File.join(main_dir, "ext"), :verbose => true)
end

def show_usage
  $stderr.puts("Usage: #{$0}")
  exit(1)
end

main
