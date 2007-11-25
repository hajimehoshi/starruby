#!/usr/bin/env ruby

require "starruby"
include StarRuby

if Font.exist?("Arial")
  font = Font.new("Arial", 12)
elsif Font.exist?("FreeFont")
  Font.exist?("FreeFont", 12)
else
  raise "font not found"
end

color = Color.new(255, 255, 255)

Game.title = "Hello, World!"

Game.run(320, 240) do
  Game.screen.clear
  Game.screen.render_text("Hello, World!", 10, 10, font, color)
end
