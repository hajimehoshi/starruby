#!/usr/bin/env ruby

require "starruby"
include StarRuby

texture = Texture.load("images/ruby")

camera = Struct.new(:x, :y, :height, :angle_n).new
camera.x = texture.width / 2
camera.y = texture.height
camera.height = texture.height / 2
camera.angle_n = 0

vanishing_x = texture.width / 2
vanishing_y = 0

screen = Texture.new(160, 120)

font = Font.new("fonts/ORANGEKI", 12)
white = Color.new(255, 255, 255)
distance = texture.height

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
  if keys.include?(:w)
    camera.height += 1
  elsif keys.include?(:z)
    camera.height -= 1
  end
  if keys.include?(:a)
    camera.angle_n = (camera.angle_n + 63) % 64
  elsif keys.include?(:s)
    camera.angle_n = (camera.angle_n + 1) % 64
  end
  if keys.include?(:d)
    vanishing_x += 1
  elsif keys.include?(:f)
    vanishing_x -= 1
  end
  if keys.include?(:c)
    vanishing_y += 1
  elsif keys.include?(:r)
    vanishing_y -= 1
  end
  
  angle = camera.angle_n * 2 * Math::PI / 64
  
  screen.fill(Color.new(64, 64, 64, 255))
  screen.render_in_perspective(texture, {
    :camera_x => camera.x,
    :camera_y => camera.y,
    :camera_height => camera.height,
    :camera_angle => angle,
    :distance => distance,
    :vanishing_x => vanishing_x,
    :vanishing_y => vanishing_y,
  })
  
  s = Game.screen
  s.clear
  s.render_texture(screen, 0, 0)
  s.render_text("(x,y,height,angle) =" % angle, 8, screen.height + 8, font, white)
  str = "(#{camera.x},#{camera.y},#{camera.height},%0.4f)" % angle
  s.render_text(str, 8, screen.height + 24, font, white)
  x = s.width  / 2 + 10
  y = s.height / 2 + 10
  s.render_texture(texture, x, y, :scale_x => 0.5, :scale_y => 0.5)
end
