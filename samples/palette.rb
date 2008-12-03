#!/usr/bin/env ruby

require "starruby"
include StarRuby

# Create Texture object with a palette
main_texture = Texture.load("images/ruby_palette", :palette => true)
# Create the palette image 
palette_texture = Texture.new(16 * 4, 16 * 4)

counter = 0
Game.run(320, 240, :title => "Palette") do |game|
  break if Input.keys(:keyboard).include?(:escape)
  # Change the palette randomly at 2 second interval
  counter += 1
  counter %= game.fps * 2
  if counter == 0
    new_palette = Array.new(main_texture.palette.size) do |i|
      Color.new(rand(256), rand(256), rand(256), main_texture.palette[i].alpha)
    end
    main_texture.change_palette!(new_palette)
  end

  # Update the palette image
  4.times do |j|
    4.times do |i|
      color = main_texture.palette[i * 4 + j]
      palette_texture.fill_rect(i * 16, j * 16, 16, 16, color)
    end
  end

  s = game.screen
  s.clear
  s.render_texture(main_texture, 16, 16, :scale_x => 2, :scale_y => 2)
  s.render_texture(palette_texture, 16 + main_texture.width * 2 + 16, 16)
end
