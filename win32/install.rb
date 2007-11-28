#!/usr/bin/env ruby

require "rbconfig"
require "fileutils"

option = {:noop => false, :verbose => true}

dlldir      = Config::CONFIG["bindir"]
sitearchdir = Config::CONFIG["sitearchdir"]

FileUtils.mkpath(dlldir)
FileUtils.mkpath(sitearchdir)

Dir.glob("dll/*.dll") {|path| FileUtils.install(path, dlldir, option)}
Dir.glob("ext/*.so")  {|path| FileUtils.install(path, sitearchdir, option)}

puts "Installation Star Ruby completed!"
