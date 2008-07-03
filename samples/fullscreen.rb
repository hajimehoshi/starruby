#!/usr/bin/env ruby

require "starruby"
include StarRuby

texture = Texture.load("images/star")
x, y = 0, 0
vx, vy = 2, 2

Game.run(240, 640, :fullscreen => true) do |game|
  break if Input.keys(:keyboard).include?(:escape)
  s = game.screen
  max_x = s.width - texture.width
  max_y = s.height - texture.height
  s.clear
  s.fill(Color.new(64, 64, 64))
  x += vx
  y += vy
  if x < 0
    x *= -1
    vx = vx.abs
  end
  if y < 0
    y *= -1
    vy = vy.abs
  end
  if max_x <= x
    x = -(x - max_x) + max_x
    vx = -(vx.abs)
  end
  if max_y <= y
    y = -(y - max_y) + max_y
    vy = -(vy.abs)
  end
  s.render_texture(texture, x, y)
end

