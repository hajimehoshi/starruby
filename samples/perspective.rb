#!/usr/bin/env ruby

require "starruby"
include StarRuby

texture = Texture.load("images/ruby")
x = texture.width / 2
y = texture.height / 2
angle_n = 0

viewport = Texture.new(160, 120)

font = Font.new("fonts/ORANGEKI", 12)
white = Color.new(255, 255, 255)

Game.title = "Perspective"

Game.run(320, 240, :window_scale => 2) do
  keys = Input.keys(:keyboard)
  if keys.include?(:left)
    x -= 1
  elsif keys.include?(:right)
    x += 1
  end
  if keys.include?(:up)
    y -= 1
  elsif keys.include?(:down)
    y += 1
  end
  if keys.include?(:a)
    angle_n = (angle_n + 63) % 64
  elsif keys.include?(:s)
    angle_n = (angle_n + 1) % 64
  end
  
  angle = angle_n * 2 * Math::PI / 64
  viewport.clear
  viewport.render_in_perspective(texture, x, y, angle, 30)
  
  s = Game.screen
  s.clear
  s.render_texture(viewport, 0, 0)
  s.render_text("(x,y,angle) =" % angle, 8, viewport.height + 8, font, white)
  s.render_text("(#{x},#{y},%0.4f)" % angle, 8, viewport.height + 24, font, white)
end
