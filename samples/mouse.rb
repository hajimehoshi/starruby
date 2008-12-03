#!/usr/bin/env ruby

require "starruby"
include StarRuby

cursor = Texture.load("images/star")

# The angle of the cursor image (degree)
angle = 0

Game.run(320, 240, :title => "Mouse (Click buttons!)") do |game|
  break if Input.keys(:keyboard).include?(:escape)
  # If 'c' key is pressed, set the mouse location at the center
  Input.mouse_location = 160, 120 if Input.keys(:keyboard).include?(:c)
  game.screen.clear
  # Get the location of the mouse cursor
  x, y = Input.mouse_location
  # Initialize properties of rendering the cursor
  alpha = 128
  scale_x = scale_y = 1
  # Get keys of the mouse
  keys = Input.keys(:mouse)
  # If the left key is pressed:
  if keys.include?(:left)
    alpha = 255
  end
  # If the right key is pressed:
  if keys.include?(:right)
    scale_x = scale_y = 2
  end
  # If the middle key is pressed:
  if keys.include?(:middle)
    angle += 6
    angle %= 360
  end
  game.screen.render_texture(cursor, x, y, {
    :alpha    => alpha,
    :center_x => cursor.width / 2,
    :center_y => cursor.height / 2,
    :angle    => angle.degrees, # Numeric#degree(s) converts degrees into radians
    :scale_x  => scale_x,
    :scale_y  => scale_y,
  })
end
