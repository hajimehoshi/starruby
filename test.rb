#!/usr/bin/env ruby

file = ARGV[0] || "test.rb"
system("ruby -I ../ -C test #{file}")
