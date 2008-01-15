#!/usr/bin/env ruby

require "starruby"
include StarRuby

texture = Texture.load("images/ruby")

star_texture = Texture.load("images/star")

point_texture = Texture.new(3, 3)
point_texture.fill(Color.new(255, 0, 0))

screen_texture = Texture.new(160, 120)

options = {
  :camera_x       => texture.width / 2,
  :camera_y       => texture.height,
  :camera_height  => texture.height / 2,
  :camera_angle_n => 0,
  :distance       => texture.height,
  :vanishing_x    => texture.width / 2,
  :vanishing_y    => 0,
  :loop           => false,
}

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
  if Input.keys(:keyboard, :duration => 1).include?(:l)
    options[:loop] = !options[:loop]
  end
  
  screen_texture.fill(Color.new(64, 64, 64, 255))
  options.merge!(:camera_angle => options[:camera_angle_n] * 2 * Math::PI / 64)
  screen_texture.render_in_perspective(texture, options)
  [[-20, -20], [-20, 20], [20, -20], [20, 20], [0, 0]].map do |dx, dy|
    x = texture.width / 2 + dx
    y = texture.height / 2 + dy
    height = (dx == 0 and dy == 0) ? 20 : 0
    Texture.transform_in_perspective(x, y, height, options)
  end.select do |x, y, scale|
    x and y and scale and 0 < scale
  end.sort do |a, b|
    a[2] <=> b[2] # scale
  end.each do |x, y, scale|
    x = x - (star_texture.width * scale) / 2
    y = y - (star_texture.height * scale)
    screen_texture.render_texture(star_texture, x, y, {
      :scale_x => scale, :scale_y => scale
    })
  end
  
  s = Game.screen
  s.clear
  s.render_texture(screen_texture, 0, 0)
  s.render_text("[Left/Right] camera_x: #{options[:camera_x]}", 8, screen_texture.height + 8, font, white)
  s.render_text("[Up/Down] camera_y: #{options[:camera_y]}", 8, screen_texture.height + 8 + 16, font, white)
  s.render_text("[W/Z] camera_height: #{options[:camera_height]}", 8, screen_texture.height + 8 + 16 * 2, font, white)
  s.render_text("[A/S] camera_angle: %0.4f" % options[:camera_angle], 8, screen_texture.height + 8 + 16 * 3, font, white)
  s.render_text("distance: #{options[:distance]}", 8, screen_texture.height + 8 + 16 * 4, font, white)
  s.render_text("[L] loop: #{options[:loop]}", 8, screen_texture.height + 8 + 16 * 5, font, white)
  s.render_text("[D/F] vanishing_x: #{options[:vanishing_x]}", screen_texture.width + 8, screen_texture.height + 8, font, white)
  s.render_text("[R/C] vanishing_y: #{options[:vanishing_y]}", screen_texture.width + 8, screen_texture.height + 8 + 16, font, white)
  
  x = s.width / 2 + (s.width / 2 - texture.width / 2) / 2
  y = (s.height / 2 - texture.height / 2) / 2
  s.render_texture(texture, x, y, :scale_x => 0.5, :scale_y => 0.5)
  s.render_texture(point_texture, x + (options[:camera_x] - 1) / 2, y + (options[:camera_y] - 1) / 2)
  
end
