#!/usr/bin/env ruby

require "starruby"
include StarRuby

font = Font.new("fonts/ORANGEKI", 12)
white = Color.new(255, 255, 255)

Game.run(320, 240, :title => "Gamepad") do |game|
  break if Input.keys(:keyboard).include?(:escape)
  s = game.screen
  s.clear
  # Get pressed keys of the gamepad
  keys = Input.keys(:gamepad)
  # Render the arrows of the keys
  s.render_text("Directions:", 8, 8,  font, white)
  directions = keys.select{|b| b.kind_of?(Symbol)}.map{|b|b.to_s}
  s.render_text(directions.join(","), 24, 24, font, white)
  # Render the buttons of the keys
  s.render_text("Buttons:", 8, 48, font, white)
  buttons = keys.select{|b| b.kind_of?(Integer)}.map{|b|b.to_s}
  s.render_text(buttons.join(","), 24, 64, font, white)
end
