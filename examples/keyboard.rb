#!/usr/bin/env ruby

require "starruby"
include StarRuby

font = Font.new("examples/fonts/ORANGEKI", 12)
white = Color.new(255, 255, 255)

Game.title = "Keyboard"

Game.run(320, 240) do
  s = Game.screen
  s.clear
  keys = Input.pressed_keys(:keyboard)
  s.render_text("Pressed Keys:", 8, 8,  font, white)
  s.render_text(keys.map{|k|k.to_s}.join(","), 24, 24,  font, white)
end
