#!/usr/bin/env ruby

require "starruby"
include StarRuby

font = Font.new("fonts/ORANGEKI", 12)
white = Color.new(255, 255, 255)

Game.run(320, 240, :title => "Keyboard") do |game|
  s = game.screen
  s.clear
  # Get keys of the keyboard as an Array object
  keys = Input.keys(:keyboard)
  s.render_text("Pressed Keys:", 8, 8,  font, white)
  # Render keys with the format "foo,bar,baz"
  s.render_text(keys.map{|k| k.to_s}.join(","), 24, 24, font, white)
end
