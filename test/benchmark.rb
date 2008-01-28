#!/usr/bin/env ruby

require "starruby"
require "benchmark"

include StarRuby

src = Texture.new(100, 100)
dst = Texture.new(319, 239)

Benchmark.bm do |x|
  x.report do
    10000.times do |i|
      x = i % dst.width
      y = i % dst.height
      dst.render_texture(src, x, y)
    end
  end
end
