#!/usr/bin/env ruby

require "starruby"
require "benchmark"

include StarRuby

Benchmark.bm do |bm|
  random_rgbas = Array.new(100000) do
    [rand(256), rand(256), rand(256), rand(256)]
  end
  bm.report("miss") do
    random_rgbas.each do |r, g, b, a|
      Color.new(r, g, b, a)
    end
  end
  bm.report("hit") do
    random_rgbas.each do |r, g, b, a|
      Color.new(0, 0, 0, 0)
    end
  end
end
