#!/usr/bin/env ruby

require "starruby"
include StarRuby

font = Font.new("fonts/ORANGEKI", 12)
white = Color.new(255, 255, 255)

music_texture = Texture.load("images/music")
sound_texture = Texture.load("images/sound")

Game.title = "Audio"

Game.run(320, 240) do
  music_alpha = Audio.playing_bgm? ? 255 : 128
  sound_alpha = 128
  keys = Input.pressed_keys(:keyboard, :duration => 1)
  if keys.include?(:m)
    if Audio.playing_bgm?
      Audio.stop_bgm
    else
      Audio.play_bgm("sounds/music")
    end
  end
  if keys.include?(:s)
    Audio.play_se("sounds/hello")
    sound_alpha = 255
  end
  s = Game.screen
  s.clear
  s.render_texture(music_texture, 16, 16, {
    :alpha => music_alpha,
    :scale_x => 4,
    :scale_y => 4,
  })
  text = "Press 'm' to #{Audio.playing_bgm? ? 'stop':'play'} music"
  s.render_text(text, 96, 32, font, white)
  s.render_texture(sound_texture, 16, 80, {
    :alpha => sound_alpha,
    :scale_x => 4,
    :scale_y => 4,
  })
  s.render_text("Press 's' to play sound", 96, 96, font, white)
end
