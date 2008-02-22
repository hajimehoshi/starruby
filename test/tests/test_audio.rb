#!/usr/bin/env ruby

require "starruby"
include StarRuby

class AudioTest < Test::Unit::TestCase

  def test_audio_type
    assert_raise TypeError do
      Audio.bgm_volume = nil
    end
    assert_raise TypeError do
      Audio.play_bgm(nil)
    end
    Audio.play_bgm("sounds/music")
    assert_raise TypeError do
      Audio.play_bgm("sounds/music", false)
    end
    [:position, :volume, :time].each do |key|
      assert_raise TypeError do
        Audio.play_bgm("sounds/music", key => false)
      end
    end
    assert_raise TypeError do
      Audio.play_se(nil)
    end
    assert_raise TypeError do
      Audio.play_se("sounds/sample", false)
    end
    [:panning, :volume, :time].each do |key|
      assert_raise TypeError do
        Audio.play_se("sounds/sample", key => false)
      end
    end
    assert_raise TypeError do
      Audio.stop_all_ses(false)
    end
    assert_raise TypeError do
      Audio.stop_all_ses(:time => false)
    end
    assert_raise TypeError do
      Audio.stop_bgm(false)
    end
    assert_raise TypeError do
      Audio.stop_bgm(:time => false)
    end
  end

  def test_bgm_volume
    assert_equal 255, Audio.bgm_volume
    (0..255).each do |volume|
      Audio.bgm_volume = volume
      assert_equal volume, Audio.bgm_volume
    end
    assert_raise ArgumentError do
      Audio.bgm_volume = -1
    end
    assert_raise ArgumentError do
      Audio.bgm_volume = 256
    end
  end

  def test_play_bgm
    (0..255).each do |volume|
      Audio.play_bgm("sounds/music", :volume => volume)
      assert_equal volume, Audio.bgm_volume
      Audio.stop_bgm
    end
    assert_raise ArgumentError do
      Audio.play_bgm("sounds/music", :volume => -1)
    end
    assert_raise ArgumentError do
      Audio.play_bgm("sounds/music", :volume => 256)
    end
  end

  def test_play_se
    (0..255).each do |volume|
      Audio.play_se("sounds/sample", :volume => volume)
      Audio.stop_all_ses
    end
    assert_raise ArgumentError do
      Audio.play_se("sounds/sample", :volume => -1)
    end
    assert_raise ArgumentError do
      Audio.play_se("sounds/sample", :volume => 256)
    end
    (-255..255).each do |volume|
      Audio.play_se("sounds/sample", :panning => volume)
      Audio.stop_all_ses
    end
    assert_raise ArgumentError do
      Audio.play_se("sounds/sample", :panning => -256)
    end
    assert_raise ArgumentError do
      Audio.play_se("sounds/sample", :panning => 256)
    end
  end
  
end