#!/usr/bin/env ruby

require "starruby"
include StarRuby

Game.title = "Rectangles"

Game.run(320, 240) do
  begin
    x1 = rand(320)
    x2 = rand(320)
    y1 = rand(240)
    y2 = rand(240)
    width  = (x1 - x2).abs
    height = (y1 - y2).abs
  end while width <= 0 or height <= 0
  x = [x1, x2].min
  y = [y1, y2].min
  color = Color.new(rand(256), rand(256), rand(256), rand(256))
  texture = Texture.new(width, height)
  texture.fill(color)
  Game.screen.render_texture(texture, x, y)
end
