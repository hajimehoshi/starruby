#!/usr/bin/env ruby

require "benchmark"
require "starruby"

include StarRuby

texture = Texture.load("images/ruby")
texture2 = Texture.new(100, 100)

Benchmark.bm do |x|
  x.report("normal") do
    1000.times do
      texture2.render_texture(texture, rand(100), rand(100))
    end
  end
  x.report("perspective") do
    1000.times do
      texture2.render_in_perspective(texture, rand(100), rand(100), rand(100), rand(100))
    end
  end
end
