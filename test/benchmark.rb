#!/usr/bin/env ruby

require "benchmark"
require "starruby"

include StarRuby

texture1 = Texture.new(100, 100)
texture2 = Texture.new(100, 100)

Benchmark.bm do |x|
=begin
  x.report("normal") do
    100000.times do |i|
      texture2.render_texture(texture1, i % 13, i % 17)
    end
  end
=end
  x.report("perspective") do
    100000.times do |i|
      texture2.render_in_perspective(texture1, {
        :camera_x => 500,
        :camera_y => 500,
        :camera_height => i % 19 + 10,
        :camera_angle => i % 23,
        :distance => i % 27,
      })
    end
  end
end
