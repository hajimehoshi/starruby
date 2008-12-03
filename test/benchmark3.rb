#!/usr/bin/env ruby

require "starruby"
require "benchmark"

include StarRuby

Benchmark.bm do |b|
  b.report("colors") do
    arr = Array.new(100000)
    arr.size.times do |i|
      arr[i] = Color.new(rand(256), rand(256), rand(256), rand(256))
    end
  end
end
