#!/usr/bin/env ruby

require "rbconfig"
require "fileutils"

option = {:noop => false, :verbose => true}

dlldir      = Config::CONFIG["bindir"]
sitearchdir = Config::CONFIG["sitearchdir"]

FileUtils.mkdir_p(dlldir)
FileUtils.mkdir_p(sitearchdir)

Dir.glob("dll/*.dll") do |path|
  if path =~ /zlib/
    next if File.exist?(File.join(dlldir, File.basename(path)))
  end
  FileUtils.install(path, dlldir, option)
end
Dir.glob("ext/*.so") {|path| FileUtils.install(path, sitearchdir, option)}

puts "Installation Star Ruby completed!"
