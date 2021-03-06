#!/usr/bin/env ruby

require "starruby"
include StarRuby

font = Font.new("fonts/ORANGEKI", 12)
white = Color.new(255, 255, 255)

music_texture = Texture.load("images/music")
sound_texture = Texture.load("images/sound")

bgm_position = 0
Game.run(320, 240, :title => "Audio") do |game|
  music_alpha = Audio.playing_bgm? ? 255 : 128
  sound_alpha = 128
  # Get pressed keys of a keyboard.
  # ':duration => 1' means that keys are pressed just now.
  keys = Input.keys(:keyboard, :duration => 1)
  # If ESC key is pressed, exit this game.
  break if keys.include?(:escape)
  # Get the current position (Fixnum) of BGM.
  # If you use this value when playing BGM, BGM can be played at middle of time.
  bgm_position = Audio.bgm_position
  # If 'm' key is pressed, play or stop BGM
  if keys.include?(:m)
    if Audio.playing_bgm?
      Audio.stop_bgm
    else
      Audio.play_bgm("sounds/music",
                     :position => bgm_position,
                     :loop => true)
    end
  end
  # If 's' key is pressed, play SE
  if keys.include?(:s)
    Audio.play_se("sounds/hello")
    sound_alpha = 255
  end
  s = game.screen
  s.clear
  s.render_texture(music_texture, 16, 16,
                   :alpha => music_alpha,
                   :scale_x => 4,
                   :scale_y => 4)
  text = "Press 'm' to #{Audio.playing_bgm? ? 'stop':'play'} music"
  s.render_text(text, 96, 32, font, white)
  s.render_text("Position: #{bgm_position}", 96, 48, font, white)
  s.render_texture(sound_texture, 16, 80,
                   :alpha => sound_alpha,
                   :scale_x => 4,
                   :scale_y => 4)
  s.render_text("Press 's' to play sound", 96, 96, font, white)
  if 0 < Audio.playing_se_count
    s.render_text("Now #{Audio.playing_se_count} SEs are playing", 96, 112, font, white)
  end
end
