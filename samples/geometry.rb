#!/usr/bin/env ruby

require "starruby"
include StarRuby

texture = Texture.load("images/ruby")
cover = Texture.new(320, 240)
cover.fill(Color.new(64, 64, 64))

font = Font.new("fonts/ORANGEKI", 12)
white = Color.new(255, 255, 255)

angle   = 0
scale_x = 0
scale_y = 0

Game.run(320, 240, :title => "Geometry") do |game|
  break if Input.keys(:keyboard).include?(:escape)
  s = game.screen
  s.clear
  keys = Input.keys(:keyboard, :duration => 1, :delay => 8, :interval => 2)
  if keys.include?(:r)
    angle   = 0
    scale_x = 0
    scale_y = 0
  end
  if keys.include?(:z)
    angle += 1
  elsif keys.include?(:x)
    angle -= 1
  end
  if keys.include?(:right)
    scale_x += 1
  elsif keys.include?(:left)
    scale_x -= 1
  end
  if keys.include?(:up)
    scale_y += 1
  elsif keys.include?(:down)
    scale_y -= 1
  end
  angle %= 50
  scale_x = [[scale_x, -10].max, 10].min
  scale_y = [[scale_y, -10].max, 10].min
  s.render_texture(texture, (s.width - texture.width) / 2, 32, {
    :angle    => angle * 2 * Math::PI / 50,
    :scale_x  => 2 ** (scale_x / 10.0),
    :scale_y  => 2 ** (scale_y / 10.0),
    :center_x => texture.width  / 2,
    :center_y => texture.height / 2,
  })
  if Input.keys(:keyboard).include?(:h)
    s.render_texture(cover, 0, 0, :alpha => 128)
    s.render_text("'z': Rotate right",                 8, 8,        font, white)
    s.render_text("'x': Rotate left",                  8, 8 + 16*1, font, white)
    s.render_text("'right': Stretch along the X-axis", 8, 8 + 16*3, font, white)
    s.render_text("'left': Shrink along the X-axis",   8, 8 + 16*4, font, white)
    s.render_text("'up': Stretch along the Y-axis",    8, 8 + 16*6, font, white)
    s.render_text("'down': Shrink along the Y-axis",   8, 8 + 16*7, font, white)
    s.render_text("'r': Reset",                        8, 8 + 16*9, font, white)
  else
    s.render_text("Click 'h' to show help", 8, 8, font, white)
  end
end
