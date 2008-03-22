#!/usr/bin/env ruby

require "starruby"
include StarRuby

field_texture = Texture.load("images/ruby-logo-R")
star_texture  = Texture.load("images/star")

Airship = Struct.new(:x, :y, :yaw, :pitch, :roll, :height, :screen_x, :screen_y)
airship = Airship.new
airship.x          = field_texture.width / 2
airship.y          = field_texture.height
airship.yaw        = 0 # degree
airship.pitch      = 0 # degree
airship.roll       = 0 # degree
airship.height     = 25
airship.screen_x   = 0
airship.screen_y   = 0
fearless = false

font = Font.new("fonts/ORANGEKI", 12)
yellow = Color.new(255, 255, 128)

Game.title = "Airship"
Game.run(320, 240) do
  if Input.keys(:keyboard, :duration => 1).include?(:f)
    fearless = ! fearless
  end
  keys = Input.keys(:keyboard)
  Game.terminate if keys.include?(:escape)
  if keys.include?(:left)
    airship.yaw = (airship.yaw + 358) % 360
    airship.roll = [airship.roll - (fearless ? 8 : 3), fearless ? -180 : -20].max
  elsif keys.include?(:right)
    airship.yaw = (airship.yaw + 2) % 360
    airship.roll = [airship.roll + (fearless ? 8 : 3), fearless ? 180 : 20].min
  else
    if 0 < airship.roll
      airship.roll = [airship.roll - (fearless ? 8 : 3), 0].max
    elsif airship.roll < 0
      airship.roll = [airship.roll + (fearless ? 8 : 3), 0].min
    end
  end
  if keys.include?(:space)
    airship.x = (airship.x + 10 * Math.sin(airship.yaw.degree)).to_i
    airship.y = (airship.y - 10 * Math.cos(airship.yaw.degree)).to_i
  end
  if keys.include?(:down)
    airship.height = [airship.height + 2, 45].min
    airship.pitch = [airship.pitch + (fearless ? 8 : 3), fearless ? 180 : 20].min
  elsif keys.include?(:up)
    airship.height = [airship.height - 2, 5].max
    airship.pitch = [airship.pitch - (fearless ? 8 : 3), fearless ? -180 : -20].max
  else
    if 0 < airship.pitch
      airship.pitch = [airship.pitch - (fearless ? 8 : 3), 0].max
    elsif airship.pitch < 0
      airship.pitch = [airship.pitch + (fearless ? 8 : 3), 0].min
    end
  end
  if keys.include?(:a)
    airship.screen_x -= 1
  elsif keys.include?(:s)
    airship.screen_x += 1
  end
  if keys.include?(:w)
    airship.screen_y -= 1
  elsif keys.include?(:z)
    airship.screen_y += 1
  end
  s = Game.screen
  s.fill(Color.new(128, 128, 128))
  options = {
    :camera_x => airship.x,
    :camera_y => airship.y,
    :camera_height => airship.height,
    :camera_yaw   => airship.yaw.degree,
    :camera_pitch => airship.pitch.degree,
    :camera_roll  => airship.roll.degree,
    :distance => field_texture.height,
    :intersection_x => s.width / 2 + airship.screen_x,
    :intersection_y => s.height / 2 + airship.screen_y,
    :loop => true,
    :blur => :background,
  }
  # ground
  s.render_in_perspective(field_texture, options)
  # stars
  [[-20, -20, 0], [-20, 20, 0], [20, -20, 0], [20, 20, 0],
   [0, 0, 20]].map do |x, y, height|
    x += field_texture.width / 2
    y += field_texture.height / 2
    Texture.transform_in_perspective(x, y, height, options)
  end.select do |x, y, scale|
    x and y and scale and 0 < scale
  end.sort do |a, b|
    a[2] <=> b[2] # scale
  end.each do |x, y, scale|
    x -= star_texture.width
    y -= star_texture.height
    next unless x.kind_of?(Fixnum) and y.kind_of?(Fixnum)
    s.render_texture(star_texture, x, y,
                     :scale_x => scale,
                     :scale_y => scale,
                     :center_x => star_texture.width / 2,
                     :center_y => star_texture.height,
                     :angle => 2 * Math::PI - options[:camera_roll])
  end
  s.render_text("[Arrow] Rotate Camera", 8, 8, font, yellow)
  s.render_text("[Space] Go Forward", 8, 8 + 16, font, yellow)
  s.render_text("[W][A][S][Z] Move Screen", 8, 8 + 32, font, yellow)
  str = "[F] Fearless? #{fearless ? '(Yes)' : '(No)'}"
  s.render_text(str, 8, 8 + 48, font, yellow)
end
