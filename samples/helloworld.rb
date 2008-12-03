#!/usr/bin/env ruby

# Require Star Ruby library
require "starruby"
# Include StarRuby module
include StarRuby

# Create Font object with a TTF file and 12 px size
font = Font.new("fonts/ORANGEKI", 12)
# Create Color object of white color (R, G, and B are all 255)
white = Color.new(255, 255, 255)

# Run game with 320 x 240 size of the screen.
# Given block are called at every frame (by default, it is called at 1/30 second intervals)
Game.run(320, 240, :title => "Hello, World!") do |game|
  # Quit this game if ESC key is pressed
  break if Input.keys(:keyboard).include?(:escape)
  # Initialize the screen.
  # The screen remains at the previous frame
  game.screen.clear
  # Render the text "Hello, World!" at (8, 8) with the created font and white color
  game.screen.render_text("Hello, World!", 8, 8, font, white)
end
