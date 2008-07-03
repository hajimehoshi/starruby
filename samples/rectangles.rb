#!/usr/bin/env ruby

require "starruby"
include StarRuby

Game.run(320, 240, :title => "Rectangles (Click to speed up!)") do |game|
  break if Input.keys(:keyboard).include?(:escape)
  if Input.keys(:mouse).include?(:left)
    game.fps = 100000
  else
    game.fps = 30
  end
  begin
    x1 = rand(320)
    x2 = rand(320)
    y1 = rand(240)
    y2 = rand(240)
  end while x1 == x2 or y1 == y2
  width  = (x1 - x2).abs
  height = (y1 - y2).abs
  x = [x1, x2].min
  y = [y1, y2].min
  color = Color.new(rand(256), rand(256), rand(256), rand(256))
  game.screen.render_rect(x, y, width, height, color)
end
