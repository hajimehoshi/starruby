#!/usr/bin/env ruby

require "starruby"
include StarRuby

font = Font.new("fonts/ORANGEKI", 12)
white = Color.new(255, 255, 255)

Game.run(320, 240, :title => "Window") do |game|
  break if Input.keys(:keyboard).include?(:escape)

  keys = Input.keys(:keyboard)
  if keys.include?(:f1)
    game.window_scale = 3 - game.window_scale
  elsif keys.include?(:f2)
    game.fullscreen = !game.fullscreen?
  end

  game.screen.clear
  game.screen.render_text("F1: toggle window scale 1 and 2", 8, 8, font, white)
  game.screen.render_text("F2: toggle full screen mode on and off", 8, 24, font, white)
end
