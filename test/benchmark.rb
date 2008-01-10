#!/usr/bin/env ruby

require "benchmark"
require "starruby"

include StarRuby

texture = Texture.load("images/ruby")
texture2 = Texture.new(100, 100)

Benchmark.bm do |x|
  x.report("normal") do
    2000.times do |i|
      texture2.render_texture(texture, i % 13, i % 17)
    end
  end
  x.report("perspective") do
    2000.times do |i|
      texture2.render_in_perspective(texture, i % 13, i % 17, i % 19 + 1, i % 23, i % 27)
    end
  end
end
