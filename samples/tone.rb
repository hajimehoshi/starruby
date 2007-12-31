#!/usr/bin/env ruby

require "starruby"
include StarRuby

texture = Texture.load("images/ruby")
cover = Texture.new(320, 240)
cover.fill(Color.new(64, 64, 64))

font = Font.new("fonts/ORANGEKI", 12)
white = Color.new(255, 255, 255)

tone_red   = 0
tone_green = 0
tone_blue  = 0
saturation = 255

Game.title = "Tone"

Game.run(320, 240) do
  s = Game.screen
  s.clear
  keys = Input.keys(:keyboard, :duration => 1, :delay => 8, :interval => 2)
  if keys.include?(:r)
    tone_red   = 0
    tone_green = 0
    tone_blue  = 0
    saturation = 255
  end
  if keys.include?(:d1)
    tone_red += 15
  elsif keys.include?(:d2)
    tone_red -= 15
  end
  if keys.include?(:q)
    tone_green += 15
  elsif keys.include?(:w)
    tone_green -= 15
  end
  if keys.include?(:a)
    tone_blue += 15
  elsif keys.include?(:s)
    tone_blue -= 15
  end
  if keys.include?(:z)
    saturation += 15
  elsif keys.include?(:x)
    saturation -= 15
  end
  tone_red   = [[tone_red,   -255].max, 255].min
  tone_green = [[tone_green, -255].max, 255].min
  tone_blue  = [[tone_blue,  -255].max, 255].min
  saturation = [[saturation, 0].max,    255].min
  s.render_texture(texture, (s.width - texture.width) / 2, 32, {
    :tone_red   => tone_red,
    :tone_green => tone_green,
    :tone_blue  => tone_blue,
    :saturation => saturation
  })
  if Input.keys(:keyboard).include?(:h)
    s.render_texture(cover, 0, 0, :alpha => 128)
    s.render_text("'1': Turn up the red part",     8, 8,         font, white)
    s.render_text("'2': Turn down the red part",   8, 8 + 16,    font, white)
    s.render_text("'q': Turn up the green part",   8, 8 + 16*3,  font, white)
    s.render_text("'w': Turn down the green part", 8, 8 + 16*4,  font, white)
    s.render_text("'a': Turn up the blue part",    8, 8 + 16*6,  font, white)
    s.render_text("'s': Turn down the blue part",  8, 8 + 16*7,  font, white)
    s.render_text("'z': Turn up the saturation",   8, 8 + 16*9,  font, white)
    s.render_text("'x': Turn down the saturation", 8, 8 + 16*10, font, white)
    s.render_text("'r': Reset",                    8, 8 + 16*12, font, white)
  else
    s.render_text("Click 'h' to show help", 8, 8, font, white)
    s.render_text("(R, G, B, S) = (#{tone_red}, #{tone_green}, #{tone_blue}, #{saturation})", 8, 200, font, white)
  end
end
