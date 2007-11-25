#!/usr/bin/env ruby

require "starruby"
include StarRuby

font = Font.new("fonts/ORANGEKI", 12)
color = Color.new(255, 255, 255)

Game.title = "Hello, World!"

Game.run(320, 240) do
  Game.screen.clear
  Game.screen.render_text("Hello, World!", 8, 8, font, color)
end
