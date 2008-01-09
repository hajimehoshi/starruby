#!/usr/bin/env ruby

require "starruby"
include StarRuby

texture = Texture.load("images/ruby")
line_texture = Texture.new(texture.width, 1)
ox, oy = (320 - texture.width) / 2, (240 - texture.height) / 2
counter = 0

Game.title = "Raster scroll"

Game.run(320, 240) do
  counter = (counter + 1) % 60
  s = Game.screen
  s.clear
  texture.height.times do |j|
    line_texture.render_texture(texture, 0, 0, :src_y => j)
    i = (10 * Math.sin((counter + j) * 2 * Math::PI / 60)).round
    s.render_texture(line_texture, ox + i, oy + j)
  end
end
