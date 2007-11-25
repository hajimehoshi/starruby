#!/usr/bin/env ruby

require "starruby"
include StarRuby

font = Font.new("fonts/ORANGEKI", 12)
white = Color.new(255, 255, 255)

Game.title = "Gamepad"

Game.run(320, 240) do
  s = Game.screen
  s.clear
  keys = Input.pressed_keys(:gamepad)
  s.render_text("Directions:", 8, 8,  font, white)
  directions = keys.select{|b| b.kind_of?(Symbol)}.map{|b|b.to_s}
  s.render_text(directions.join(","), 24, 24,  font, white)
  
  s.render_text("Buttons:", 8, 48, font, white)
  buttons = keys.select{|b| b.kind_of?(Integer)}.map{|b|b.to_s}
  s.render_text(buttons.join(","), 24, 64,  font, white)
end
