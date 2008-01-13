#!/usr/bin/env ruby

require "benchmark"
require "starruby"

include StarRuby

texture1 = Texture.new(1000, 1000)
texture2 = Texture.new(1000, 1000)

Benchmark.bm do |x|
  x.report("normal") do
    100.times do |i|
      texture2.render_texture(texture1, i % 13, i % 17)
    end
  end
  x.report("perspective") do
    100.times do |i|
      texture2.render_in_perspective(texture1, 500, 500, i % 19 + 10, i % 23, i % 27)
    end
  end
end
