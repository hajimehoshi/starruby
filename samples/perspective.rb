#!/usr/bin/env ruby

require "starruby"
include StarRuby

texture = Texture.load("images/ruby")

camera = Struct.new(:x, :y, :angle_n).new
camera.x = texture.width / 2
camera.y = texture.height / 2
camera.angle_n = 0

viewport = Texture.new(160, 120)

font = Font.new("fonts/ORANGEKI", 12)
white = Color.new(255, 255, 255)

Game.title = "Perspective"

i = 0
Game.run(320, 240, :window_scale => 2) do
  i = (i + 1) % 30
  keys = Input.keys(:keyboard)
  if keys.include?(:left)
    camera.x -= 1
  elsif keys.include?(:right)
    camera.x += 1
  end
  if keys.include?(:up)
    camera.y -= 1
  elsif keys.include?(:down)
    camera.y += 1
  end
  if keys.include?(:a)
    camera.angle_n = (camera.angle_n + 63) % 64
  elsif keys.include?(:s)
    camera.angle_n = (camera.angle_n + 1) % 64
  end
  
  angle = camera.angle_n * 2 * Math::PI / 64
  viewport.clear
  viewport.render_in_perspective(texture, camera.x, camera.y, angle, 30)
  
  s = Game.screen
  s.clear
  s.render_texture(viewport, 0, 0)
  s.render_text("(x,y,angle) =" % angle, 8, viewport.height + 8, font, white)
  str = "(#{camera.x},#{camera.y},%0.4f)" % angle
  s.render_text(str, 8, viewport.height + 24, font, white)
  x = s.width  / 2 + 10
  y = s.height / 2 + 10
  s.render_texture(texture, x, y, :scale_x => 0.5, :scale_y => 0.5)
end
