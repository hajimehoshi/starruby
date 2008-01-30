#!/usr/bin/env ruby

require "starruby"
require "benchmark"

include StarRuby

src = Texture.new(100, 100)
src.undump(Array.new(src.width * src.height * 3){rand(256).chr}.join, "rgb")
src.undump(Array.new(src.width * src.height){0 < rand(2) ? "\xff" : "\x00"}.join, "a")
dst = Texture.new(319, 239)
dst.undump(Array.new(dst.width * dst.height * 3){rand(256).chr}.join, "rgb")
dst.undump(Array.new(dst.width * dst.height){rand(256).chr}.join, "a")

Benchmark.bm do |b|
  dst.clear
  b.report "normal" do
    10000.times do |i|
      x = i % dst.width
      y = i % dst.height
      dst.render_texture(src, x, y)
    end
  end
  dst.clear
  b.report "alpha " do
    10000.times do |i|
      x = i % dst.width
      y = i % dst.height
      alpha = i % 256
      dst.render_texture(src, x, y, :alpha => i)
    end
  end
  dst.clear
  b.report "scale " do
    10000.times do |i|
      x = i % dst.width
      y = i % dst.height
      dst.render_texture(src, x, y, :scale_x => 2)
    end
  end
  dst.clear
  b.report "tone  " do
    10000.times do |i|
      x = i % dst.width
      y = i % dst.height
      dst.render_texture(src, x, y, :tone_red => 100)
    end
  end
  dst.clear
  b.report "sub   " do
    10000.times do |i|
      x = i % dst.width
      y = i % dst.height
      dst.render_texture(src, x, y, :blend_type => :sub)
    end
  end
end
