#!/usr/bin/env ruby

require "starruby"
include StarRuby

font = Font.new("fonts/ORANGEKI", 24)
white = Color.new(255, 255, 255)

Game.title = "Timer"
counter = 0
Game.run(320, 240) do
  Game.terminate if Input.keys(:keyboard).include?(:escape)

  counter += 1

  s = Game.screen
  s.clear
  i = -1
  s.render_text("FPS: #{Game.fps}",                8, 8 + 32 * (i += 1), font, white)
  s.render_text("Real FPS: %0.6f" % Game.real_fps, 8, 8 + 32 * (i += 1), font, white)
  s.render_text("Frames: #{counter}",              8, 8 + 32 * (i += 1), font, white)
  s.render_text("Ticks: #{Game.ticks}",            8, 8 + 32 * (i += 1), font, white)
end

