#!/usr/bin/env ruby

require "starruby"
include StarRuby

texture = Texture.load("images/ruby")
ox, oy = (320 - texture.width) / 2, (240 - texture.height) / 2
counter = 0

Game.title = "Raster scroll"

Game.run(320, 240) do
  Game.terminate if Input.keys(:keyboard).include?(:escape)
  counter = (counter + 1) % 60
  s = Game.screen
  s.clear
  texture.height.times do |j|
    i = (10 * Math.sin((counter + j) * 2 * Math::PI / 60)).round
    s.render_texture(texture, ox + i, oy + j, {
      :src_x => 0, :src_y => j, :src_height => 1
    })
  end
end
