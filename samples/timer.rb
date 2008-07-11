#!/usr/bin/env ruby

require "starruby"
include StarRuby

font = Font.new("fonts/ORANGEKI", 24)
white = Color.new(255, 255, 255)

counter = 0
Game.run(320, 240, :timer => "Timer") do |game|
  break if Input.keys(:keyboard).include?(:escape)

  counter += 1

  s = game.screen
  s.clear
  i = -1
  s.render_text("FPS: #{game.fps}",                8, 8 + 32 * (i += 1), font, white)
  s.render_text("Real FPS: %0.6f" % game.real_fps, 8, 8 + 32 * (i += 1), font, white)
  s.render_text("Frames: #{counter}",              8, 8 + 32 * (i += 1), font, white)
  s.render_text("Ticks: #{Game.ticks}",            8, 8 + 32 * (i += 1), font, white)
end
