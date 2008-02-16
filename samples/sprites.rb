#!/usr/bin/env ruby

require "starruby"
include StarRuby

class Sprite
  @@texture = Texture.load("images/star")
  MAX_X = 320 - @@texture.width
  MAX_Y = 240 - @@texture.height
  
  attr_reader :x
  attr_reader :y
  
  def initialize
    @x = rand(MAX_X)
    @y = rand(MAX_Y)
    @vx = rand(2) * 2 - 1
    @vy = rand(2) * 2 - 1
  end
  
  def texture
    @@texture
  end
  
  def update
    @x += @vx
    @y += @vy
    if @x < 0
      @x = -@x
      @vx = 1
    end
    if @y < 0
      @y = -@y
      @vy = 1
    end
    if MAX_X <= @x
      @x = -(@x - MAX_X) + MAX_X
      @vx = -1
    end
    if MAX_Y <= @y
      @y = -(@y - MAX_Y) + MAX_Y
      @vy = -1
    end
  end
end

sprites = Array.new(200) {Sprite.new}

Game.title = "Sprites (Click to speed up!)"

fps_text_texture = Texture.new(320, 64)
Game.run(320, 240, :window_scale => 2) do
  Game.terminate if Input.keys(:keyboard).include?(:escape)
  if Input.keys(:mouse).include?(:left)
    Game.fps = 100000
  else
    Game.fps = 30
  end
  sprites.each do |sprite|
    sprite.update
  end
  Game.screen.clear
  sprites.each do |sprite|
    Game.screen.render_texture(sprite.texture, sprite.x, sprite.y)
  end
end
