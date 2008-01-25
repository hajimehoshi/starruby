#!/usr/bin/env ruby

require "starruby"
include StarRuby

field_texture = Texture.load("images/ruby-logo-R")

Airship = Struct.new(:x, :y, :yaw_int, :pitch_int, :roll_int, :height)
airship = Airship.new
airship.x          = field_texture.width / 2
airship.y          = field_texture.height
airship.yaw_int    = 0
airship.pitch_int  = 0
airship.roll_int   = 0
airship.height     = 25
fearless = false

font = Font.new("fonts/ORANGEKI", 12)
yellow = Color.new(255, 255, 128)

class Numeric
  def to_radian
    self * 2 * Math::PI / 128
  end
end

Game.title = "Airship"
Game.run(320, 240) do
  if Input.keys(:keyboard, :duration => 1).include?(:f)
    fearless = ! fearless
  end
  keys = Input.keys(:keyboard)
  Game.terminate if keys.include?(:escape)
  if keys.include?(:left)
    airship.yaw_int = (airship.yaw_int + 127) % 128
    
    airship.roll_int = [airship.roll_int - (fearless ? 4 : 1), fearless ? -64 : -8].max
  elsif keys.include?(:right)
    airship.yaw_int = (airship.yaw_int + 1) % 128
    airship.roll_int = [airship.roll_int + (fearless ? 4 : 1), fearless ? 64 : 8].min
  else
    if 0 < airship.roll_int
      airship.roll_int -= (fearless ? 4 : 1)
    elsif airship.roll_int < 0
      airship.roll_int += (fearless ? 4 : 1)
    end
  end
  yaw_radian = airship.yaw_int.to_radian
  if keys.include?(:space)
    airship.x = (airship.x + 10 * Math.sin(yaw_radian)).to_i
    airship.y = (airship.y - 10 * Math.cos(yaw_radian)).to_i
  end
  if keys.include?(:down)
    airship.height = [airship.height + 2, 45].min
    airship.pitch_int = [airship.pitch_int + (fearless ? 4 : 1), fearless ? 64 : 8].min
  elsif keys.include?(:up)
    airship.height = [airship.height - 2, 5].max
    airship.pitch_int = [airship.pitch_int - (fearless ? 4 : 1), fearless ? -64 : -8].max
  else
    if 0 < airship.pitch_int
      airship.pitch_int -= (fearless ? 4 : 1)
    elsif airship.pitch_int < 0
      airship.pitch_int += (fearless ? 4 : 1)
    end
  end
  s = Game.screen
  s.fill(Color.new(128, 128, 128))
  options = {
    :camera_x => airship.x,
    :camera_y => airship.y,
    :camera_height => airship.height,
    :camera_angle => {
      :yaw   => yaw_radian,
      :pitch => airship.pitch_int.to_radian,
      :roll  => airship.roll_int.to_radian,
    },
    :distance => field_texture.height / 2,
    :intersection_x => s.width / 2,
    :intersection_y => s.height / 2,
    :loop => true,
  }
  s.render_in_perspective(field_texture, options)
  s.render_text("[Arrow] Rotate Camera", 8, 8, font, yellow)
  s.render_text("[Space] Go Forward", 8, 8 + 16, font, yellow)
  str = "[F] Fearless? #{fearless ? '(Yes)' : '(No)'}"
  s.render_text(str, 8, 8 + 32, font, yellow)
end
