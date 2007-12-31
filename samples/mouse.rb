#!/usr/bin/env ruby

require "starruby"
include StarRuby

cursor = Texture.load("images/star")

angle = 0

Game.title = "Mouse (Click buttons!)"

Game.run(320, 240) do
  Game.screen.clear
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
    angle += 1
    angle %= 60
  end
  Game.screen.render_texture(cursor, x, y, {
    :alpha => alpha,
    :center_x => cursor.width / 2,
    :center_y => cursor.height / 2,
    :angle => 2 * Math::PI * angle / 60,
    :scale_x => scale_x,
    :scale_y => scale_y,
  })
end
