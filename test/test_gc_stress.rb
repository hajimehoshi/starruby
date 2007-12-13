#!/usr/bin/env ruby

require "starruby"
require "test/unit"

GC.stress = true if GC.respond_to?(:stress=)

include StarRuby

class GCStressTest < Test::Unit::TestCase
  
  def test_color
    color = Color.new(1, 2, 3, 4)
    color.red
    color.green
    color.blue
    color.alpha
    color == Color.new(1, 2, 3, 4)
    color.hash
    color.eql?(Color.new(1, 2, 3, 4))
    color.to_s
  end
  
  def test_font
    Font.exist?("Arial")
    Font.exist?("FreeSans")
    Font.exist?("fonts/ORANGEKI")
    font = Font.new("fonts/ORANGEKI", 12, {
      :bold => true, :italic => true, :ttc_index => 0
    })
    font.name
    font.bold?
    font.italic?
    font.size
    font.get_size("foo")
    font.dispose
    font.disposed?
  end

  def test_game
    Game.fps
    Game.fps = 30
    Game.title
    Game.title = "foo"
    Game.screen
    Game.running?
    Game.terminate
    Game.run(320, 240) do
      Game.fps
      Game.fps = 30
      Game.title
      Game.title = "foo"
      Game.screen
      Game.running?
      Game.terminate
    end
  end

  def test_texture
    t = Texture.new(123, 456)
    t.size
    t.width
    t.height
    font = Font.new("fonts/ORANGEKI", 12)
    color = Color.new(255, 255, 255)
    t = Texture.new_text("foo", font, color, true)
    t = Texture.load("images/ruby")
    t = t.clone
    t = t.dup
    t.get_pixel(0, 1)
    t.set_pixel(2, 3, Color.new(4, 5, 6, 7))
    t.fill(Color.new(0, 1, 2, 3))
    t.fill_rect(0, 1, 2, 3, Color.new(4, 5, 6, 7))
    t = t.change_hue(Math::PI)
    t.change_hue!(Math::PI)
    t.render_text("bar", 0, 1, font, color)
    t.render_texture(t, 0, 1, {
      :alpha => 2,
      :angle => Math::PI * 3 / 4,
      :blend_type => :add,
      :center_x => 3,
      :center_y => 4,
      :saturation => 5,
      :scale_x => 6,
      :scale_y => 7,
      :src_height => 9,
      :src_width => 8,
      :src_x => 10,
      :src_y => 11,
      :tone_red => 12,
      :tone_green => -13,
      :tone_blue => 14,
    })
    t.clear
    t.save("images/saved_image.png")
    if FileTest.exist?("images/saved_image.png")
      File.delete("images/saved_image.png")
    end
    str = t.dump("rgb")
    t.undump(str, "gbr")
    t.dispose
    t.disposed?
  end
  
  def test_input
    Input.pressed_keys(:keyboard, :duration => 0, :delay => 1, :interval => 2)
    Input.pressed_keys(:gamepad, {
      :device_number => 3, :duration => 4, :delay => 5, :interval => 6
    })
    Input.pressed_keys(:mouse, :duration => 7, :delay => 8, :interval => 9)
    Input.mouse_location
  end
  
  def test_audio
    Audio.bgm_volume
    Audio.bgm_volume = 0
    Audio.play_bgm("sounds/music")
    Audio.playing_bgm?
    Audio.stop_bgm
    Audio.playing_bgm?
    Audio.play_bgm("sounds/music", :position => 0, :volume => 1, :time => 2)
    Audio.playing_bgm?
    Audio.stop_bgm(:time => 3)
    Audio.play_se("sounds/sample")
    Audio.stop_all_ses
    Audio.play_se("sounds/sample", :panning => 4, :volume => 5, :time => 6)
    Audio.stop_all_ses(:time => 7)
  end
  
end
