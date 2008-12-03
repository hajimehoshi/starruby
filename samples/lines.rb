#!/usr/bin/env ruby

require "starruby"
include StarRuby

class Texture
  # Add a method of StarRuby::Texture class
  def render_line_16(x1, y1, x2, y2, color)
    render_line(x1 * 16, y1 * 16, x2 * 16, y2 * 16, color)
  end
end

color = Color.new(255, 255, 255, 255)

Game.run(320, 240, :title => "Lines") do |game|
  break if Input.keys(:keyboard).include?(:escape)
  s = game.screen
  s.clear

  # S
  s.render_line_16(4, 1, 2, 1, color)
  s.render_line_16(2, 1, 1, 1.5, color)
  s.render_line_16(1, 1.5, 2, 2.5, color)
  s.render_line_16(2, 2.5, 3, 2.5, color)
  s.render_line_16(3, 2.5, 4, 3, color)
  s.render_line_16(4, 3, 3, 4, color)
  s.render_line_16(3, 4, 1, 4, color)
  
  # T
  s.render_line_16(5, 1, 8, 1, color)
  s.render_line_16(6.5, 1, 6.5, 4, color)

  # A
  s.render_line_16(10.5, 1, 9, 3, color)
  s.render_line_16(9, 3, 9, 4, color)
  s.render_line_16(10.5, 1, 12, 3, color)
  s.render_line_16(12, 3, 12, 4, color)
  s.render_line_16(9, 3, 12, 3, color)

  # R
  s.render_line_16(13, 1, 15, 1, color)
  s.render_line_16(15, 1, 16, 2, color)
  s.render_line_16(13, 3, 15, 3, color)
  s.render_line_16(15, 3, 16, 2, color)
  s.render_line_16(13, 1, 13, 4, color)
  s.render_line_16(15, 3, 16, 4, color)

  # R
  s.render_line_16(1, 5, 3, 5, color)
  s.render_line_16(3, 5, 4, 6, color)
  s.render_line_16(1, 7, 3, 7, color)
  s.render_line_16(3, 7, 4, 6, color)
  s.render_line_16(1, 5, 1, 8, color)
  s.render_line_16(3, 7, 4, 8, color)

  # U
  s.render_line_16(5, 5, 5, 7, color)
  s.render_line_16(5, 7, 6.5, 8, color)
  s.render_line_16(6.5, 8, 8, 7, color)
  s.render_line_16(8, 7, 8, 5, color)

  # B
  s.render_line_16(9, 5, 9, 8, color)
  s.render_line_16(9, 5, 11, 5, color)
  s.render_line_16(9, 6.5, 11, 6.5, color)
  s.render_line_16(9, 8, 11, 8, color)
  s.render_line_16(11, 5, 12, 5.5, color)
  s.render_line_16(12, 5.5, 11, 6.5, color)
  s.render_line_16(11, 6.5, 12, 7, color)
  s.render_line_16(12, 7, 11, 8, color)

  # Y
  s.render_line_16(13, 5, 14.5, 7, color)
  s.render_line_16(16, 5, 14.5, 7, color)
  s.render_line_16(14.5, 7, 14.5, 8, color)
end
