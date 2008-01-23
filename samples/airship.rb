#!/usr/bin/env ruby

require "starruby"
include StarRuby

Airship = Struct.new(:x, :y, :yaw, :height)
airship = Airship.new
airship.x      = 0
airship.y      = 0
airship.yaw    = 0
airship.height = 25

field_texture = Texture.load("images/ruby-logo-R")

Game.title = "Airship"
Game.run(320, 240) do
  keys = Input.keys(:keyboard)
  if keys.include?(:left)
    airship.yaw = (airship.yaw + 63) % 64
  elsif keys.include?(:right)
    airship.yaw = (airship.yaw + 1) % 64
  end
  yaw_radian = airship.yaw * 2 * Math::PI / 64
  if keys.include?(:space)
    airship.x = (airship.x + 20 * Math.sin(yaw_radian)).to_i
    airship.y = (airship.y - 20 * Math.cos(yaw_radian)).to_i
  end
  if keys.include?(:down)
    airship.height = [airship.height + 2, 55].min
  elsif keys.include?(:up)
    airship.height = [airship.height - 2, 5].max
  end
  s = Game.screen
  s.fill(Color.new(128, 128, 128))
  options = {
    :camera_x => airship.x,
    :camera_y => airship.y,
    :camera_height => airship.height,
    :camera_angle => {
      :yaw   => yaw_radian,
      :pitch => 0,
      :roll  => 0,
    },
    :distance => field_texture.height / 2,
    :intersection_x => s.width / 2,
    :intersection_y => s.height / 2,
    :loop => true,
  }
  s.render_in_perspective(field_texture, options)
end
