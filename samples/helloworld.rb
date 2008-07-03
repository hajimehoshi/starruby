#!/usr/bin/env ruby

require "starruby"
include StarRuby

font = Font.new("fonts/ORANGEKI", 12)
white = Color.new(255, 255, 255)

Game.run(320, 240, :title => "Hello, World!") do |game|
  break if Input.keys(:keyboard).include?(:escape)
  game.screen.clear
  game.screen.render_text("Hello, World!", 8, 8, font, white)
end
