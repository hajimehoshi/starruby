#!/usr/bin/env ruby

require "starruby"
include StarRuby

field_texture = Texture.load("images/ruby-logo-R")
star_texture  = Texture.load("images/star")

Airship = Struct.new(:x, :y, :yaw, :pitch, :roll, :height)
airship = Airship.new
airship.x      = field_texture.width / 2
airship.y      = field_texture.height
airship.yaw    = 0
airship.pitch  = 0
airship.roll   = 0
airship.height = 25
fearless = false

font = Font.new("fonts/ORANGEKI", 12)
yellow = Color.new(255, 255, 128)

Game.run(320, 240, :title => "Airship") do |game|
  # Begin Inputing
  if Input.keys(:keyboard, :duration => 1).include?(:f)
    fearless = ! fearless
  end
  keys = Input.keys(:keyboard)
  # Terminate if the ESC key is pressed
  break if keys.include?(:escape)
  # Go left or right
  if keys.include?(:left)
    airship.yaw = (airship.yaw + 358) % 360
    airship.roll = [airship.roll - (fearless ? 8 : 3), fearless ? -180 : -20].max
  elsif keys.include?(:right)
    airship.yaw = (airship.yaw + 2) % 360
    airship.roll = [airship.roll + (fearless ? 8 : 3), fearless ? 180 : 20].min
  else
    # If not pressed the left or the right key,
    # the airship will be set its original position gradually.
    if 0 < airship.roll
      airship.roll = [airship.roll - (fearless ? 8 : 3), 0].max
    elsif airship.roll < 0
      airship.roll = [airship.roll + (fearless ? 8 : 3), 0].min
    end
  end
  # Go forward
  if keys.include?(:space)
    airship.x = (airship.x + 10 * Math.sin(airship.yaw.degree)).to_i
    airship.y = (airship.y - 10 * Math.cos(airship.yaw.degree)).to_i
  end
  # Go up or down
  if keys.include?(:down)
    airship.height = [airship.height + 2, 45].min
    airship.pitch = [airship.pitch + (fearless ? 8 : 3), fearless ? 180 : 20].min
  elsif keys.include?(:up)
    airship.height = [airship.height - 2, 5].max
    airship.pitch = [airship.pitch - (fearless ? 8 : 3), fearless ? -180 : -20].max
  else
    # If not pressed the up or the down key,
    # the airship will be set its original position gradually.
    if 0 < airship.pitch
      airship.pitch = [airship.pitch - (fearless ? 8 : 3), 0].max
    elsif airship.pitch < 0
      airship.pitch = [airship.pitch + (fearless ? 8 : 3), 0].min
    end
  end
  # End Inputing
  # Begin View
  s = game.screen
  s.fill(Color.new(128, 128, 128))
  options = {
    :camera_x => airship.x,
    :camera_y => airship.y,
    :camera_height => airship.height,
    :camera_yaw   => airship.yaw.degree,
    :camera_pitch => airship.pitch.degree,
    :camera_roll  => airship.roll.degree,
    :loop => true,
    :blur => :background,
  }
  # Render the ground
  s.render_in_perspective(field_texture, options)
  # Render stars
  # The following array means stars' coords ([x, y, height]).
  [[-20, -20, 0], [-20, 20, 0], [20, -20, 0], [20, 20, 0],
   [0, 0, 20]].map do |x, y, height|
    x += field_texture.width / 2
    y += field_texture.height / 2
    # The following statement returns an array ([x, y, scale])
    s.transform_in_perspective(x, y, height, options)
    # The following code was deprecated since the version 0.2
    # Texture.transform_in_perspective(x, y, height, options)
  end.select do |x, y, scale|
    # If either x, y or scale holds nil value,
    # the star will not be shown.
    x and y and scale and 0 < scale
  end.sort do |a, b|
    # Sort by each scale
    a[2] <=> b[2]
  end.each do |x, y, scale|
    # Adjust the rendering position
    x -= star_texture.width / 2
    y -= star_texture.height
    # If x or y value is not Fixnum, skip rendering.
    next unless x.kind_of?(Fixnum) and y.kind_of?(Fixnum)
    s.render_texture(star_texture, x, y,
                     :scale_x => scale,
                     :scale_y => scale,
                     # A star's center is its lower center.
                     :center_x => star_texture.width / 2,
                     :center_y => star_texture.height,
                     :angle => 2 * Math::PI - options[:camera_roll])
  end
  # Render texts
  s.render_text("[Arrow] Rotate Camera", 8, 8, font, yellow)
  s.render_text("[Space] Go Forward", 8, 8 + 16, font, yellow)
  str = "[F] Fearless? #{fearless ? '(Yes)' : '(No)'}"
  s.render_text(str, 8, 8 + 32, font, yellow)
  # End Viewing
end
