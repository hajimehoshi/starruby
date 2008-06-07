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
  b.report "loop  " do
    10000.times do |i|
      dst.clear if i & 3 == 0
    end
  end
  b.report "normal" do
    10000.times do |i|
      dst.clear if i & 3 == 0
      x = i % dst.width
      y = i % dst.height
      dst.render_texture(src, x, y)
    end
  end
  dst.clear
  b.report "alpha " do
    10000.times do |i|
      dst.clear if i & 3 == 0
      x = i % dst.width
      y = i % dst.height
      alpha = i % 256
      dst.render_texture(src, x, y, :alpha => i)
    end
  end
  dst.clear
  b.report "none  " do
    10000.times do |i|
      dst.clear if i & 3 == 0
      x = i % dst.width
      y = i % dst.height
      dst.render_texture(src, x, y, :blend_type => :none)
    end
  end
  dst.clear
  angle = Math::PI / 4
  b.report "geo   " do
    10000.times do |i|
      dst.clear if i & 3 == 0
      x = i % dst.width
      y = i % dst.height
      dst.render_texture(src, x, y, :scale_x => 2, :angle => angle)
    end
  end
  dst.clear
  b.report "tone  " do
    10000.times do |i|
      dst.clear if i & 3 == 0
      x = i % dst.width
      y = i % dst.height
      dst.render_texture(src, x, y, :tone_red => 100)
    end
  end
  dst.clear
  b.report "sub   " do
    10000.times do |i|
      dst.clear if i & 3 == 0
      x = i % dst.width
      y = i % dst.height
      dst.render_texture(src, x, y, :blend_type => :sub)
    end
  end
  dst.clear
  b.report "perse " do
    1000.times do |i|
      dst.clear if i & 3 == 0
      dst.render_in_perspective(src,
                                :camera_x => src.width / 2,
                                :camera_y => src.height,
                                :camera_height => 100,
                                :intersection_x => dst.width / 2,
                                :intersection_y => dst.height / 2,
                                :loop => true)
    end
  end
  dst.clear
  b.report "perse2" do
    1000.times do |i|
      dst.clear if i & 3 == 0
      dst.render_in_perspective(src,
                                :camera_x => src.width / 2,
                                :camera_y => src.height,
                                :camera_height => 100,
                                :intersection_x => dst.width / 2,
                                :intersection_y => dst.height / 2,
                                :loop => true,
                                :blur => Color.new(0, 0, 0))
    end
  end
end
