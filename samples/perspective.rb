#!/usr/bin/env ruby

require "starruby"
include StarRuby

texture = Texture.load("images/ruby")
point_texture = Texture.new(3, 3)
point_texture.fill(Color.new(255, 255, 0))

options = {
  :camera_x       => texture.width / 2,
  :camera_y       => texture.height,
  :camera_height  => texture.height / 2,
  :camera_angle_n => 0,
  :distance       => texture.height,
  :vanishing_x    => texture.width / 2,
  :vanishing_y    => 0,
}

screen = Texture.new(160, 120)

font = Font.new("fonts/ORANGEKI", 12)
white = Color.new(255, 255, 255)

Game.title = "Perspective"

i = 0
Game.run(320, 240, :window_scale => 2) do
  i = (i + 1) % 30
  keys = Input.keys(:keyboard)
  if keys.include?(:left)
    options[:camera_x] -= 1
  elsif keys.include?(:right)
    options[:camera_x] += 1
  end
  if keys.include?(:up)
    options[:camera_y] -= 1
  elsif keys.include?(:down)
    options[:camera_y] += 1
  end
  if keys.include?(:w)
    options[:camera_height] += 1
  elsif keys.include?(:z)
    options[:camera_height] -= 1
  end
  if keys.include?(:a)
    options[:camera_angle_n] = (options[:camera_angle_n] + 63) % 64
  elsif keys.include?(:s)
    options[:camera_angle_n] = (options[:camera_angle_n] + 1) % 64
  end
  if keys.include?(:f)
    options[:vanishing_x] += 1
  elsif keys.include?(:d)
    options[:vanishing_x] -= 1
  end
  if keys.include?(:c)
    options[:vanishing_y] += 1
  elsif keys.include?(:r)
    options[:vanishing_y] -= 1
  end
  
  angle = options[:camera_angle_n] * 2 * Math::PI / 64
  
  screen.fill(Color.new(64, 64, 64, 255))
  screen.render_in_perspective(texture, options.merge(:camera_angle => angle))
  
  s = Game.screen
  s.clear
  s.render_texture(screen, 0, 0)
  s.render_text("[Left/Right] camera_x: #{options[:camera_x]}", 8, screen.height + 8, font, white)
  s.render_text("[Up/Down] camera_y: #{options[:camera_y]}", 8, screen.height + 8 + 16, font, white)
  s.render_text("[W/Z] camera_height: #{options[:camera_height]}", 8, screen.height + 8 + 16 * 2, font, white)
  s.render_text("[A/S] camera_angle: %0.4f" % angle, 8, screen.height + 8 + 16 * 3, font, white)
  s.render_text("distance: #{options[:distance]}" % angle, 8, screen.height + 8 + 16 * 4, font, white)
  s.render_text("[D/F] vanishing_x: #{options[:vanishing_x]}", screen.width + 8, screen.height + 8, font, white)
  s.render_text("[R/C] vanishing_y: #{options[:vanishing_y]}", screen.width + 8, screen.height + 8 + 16, font, white)
  
  x = s.width / 2 + (s.width / 2 - texture.width / 2) / 2
  y = (s.height / 2 - texture.height / 2) / 2
  s.render_texture(texture, x, y, :scale_x => 0.5, :scale_y => 0.5)
  s.render_texture(point_texture, x + (options[:camera_x] - 1) / 2, y + (options[:camera_y] - 1) / 2)
  
end
