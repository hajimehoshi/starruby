#!/usr/bin/env ruby

require "starruby"
require "benchmark"

include StarRuby

src = Texture.new(100, 100)
src.undump(Array.new(src.width * src.height * 3){rand(256).chr}.join, "rgb")
src.undump(Array.new(src.width * src.height){rand(2) ? "\x00" : "\xff"}.join, "a")
dst = Texture.new(319, 239)
dst.undump(Array.new(dst.width * dst.height * 3){rand(256).chr}.join, "rgb")
dst.undump(Array.new(dst.width * dst.height){rand(256).chr}.join, "a")

Benchmark.bm do |x|
  x.report do
    10000.times do |i|
      x = i % dst.width
      y = i % dst.height
      dst.render_texture(src, x, y)
    end
  end
end
