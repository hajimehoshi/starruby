#!/usr/bin/env ruby

require "starruby"
include StarRuby

cursor = Texture.load("images/star")

angle = 0

Game.run(320, 240, :title => "Mouse (Click buttons!)") do |game|
  break if Input.keys(:keyboard).include?(:escape)
  Input.mouse_location = 160, 120 if Input.keys(:keyboard).include?(:c)
  game.screen.clear
  x, y = Input.mouse_location
  alpha = 128
  scale_x = scale_y = 1
  keys = Input.keys(:mouse)
  if keys.include?(:left)
    alpha = 255
  end
  if keys.include?(:right)
    scale_x = scale_y = 2
  end
  if keys.include?(:middle)
    angle += 6
    angle %= 360
  end
  game.screen.render_texture(cursor, x, y, {
    :alpha => alpha,
    :center_x => cursor.width / 2,
    :center_y => cursor.height / 2,
    :angle => angle.degrees,
    :scale_x => scale_x,
    :scale_y => scale_y,
  })
end
