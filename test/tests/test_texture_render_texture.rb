#!/usr/bin/env ruby

require "test/unit"
require "starruby"
include StarRuby

require "frozen_error"

class TestTextureRenderTexture < Test::Unit::TestCase

  def test_render_texture
    texture = Texture.load("images/ruby")
    texture2 = Texture.new(texture.width, texture.height)
    texture2.render_texture(texture, 0, 0)
    texture.height.times do |j|
      texture.width.times do |i|
        c1 = texture[i, j]
        c2 = texture2[i, j]
        assert_equal c1, c2
      end
    end
    texture2.fill(Color.new(128, 128, 128, 128))
    texture2.render_texture(texture, 0, 0)
    texture.height.times do |j|
      texture.width.times do |i|
        p1 = texture[i, j]
        p2 = texture2[i, j]
        a = p1.alpha
        assert_in_delta (p1.red   * a + 128 * (255 - a)).quo(255), p2.red,   2
        assert_in_delta (p1.green * a + 128 * (255 - a)).quo(255), p2.green, 2
        assert_in_delta (p1.blue  * a + 128 * (255 - a)).quo(255), p2.blue,  2
        assert_equal [p1.alpha, 128].max, p2.alpha
      end
    end
  end
  
  def test_render_texture_frozen
    texture = Texture.load("images/ruby")
    texture2 = Texture.new(texture.width, texture.height)
    texture2.freeze
    assert_raise FrozenError do
      texture2.render_texture(texture, 0, 0)
    end
  end
  
  def test_render_texture_disposed
    texture = Texture.load("images/ruby")
    texture2 = Texture.new(texture.width, texture.height)
    texture2.dispose
    assert_raise RuntimeError do
      texture2.render_texture(texture, 0, 0)
    end
    texture3 = Texture.new(texture.width, texture.height)
    texture.dispose
    assert_raise RuntimeError do
      texture3.render_texture(texture, 0, 0)
    end
  end
  
  def test_render_texture_type
    texture = Texture.load("images/ruby")
    texture2 = Texture.new(texture.width, texture.height)
    assert_raise TypeError do
      texture2.render_texture(nil, 0, 0, {})
    end
    assert_raise TypeError do
      texture2.render_texture(texture, nil, 0, {})
    end
    assert_raise TypeError do
      texture2.render_texture(texture, 0, nil, {})
    end
    assert_raise TypeError do
      texture2.render_texture(texture, 0, 0, false)
    end
    [:alpha, :angle, :blend_type, :center_x, :center_y, :matrix,
     :saturation, :scale_x, :scale_y, :src_height, :src_width,
     :src_x, :src_y, :tone_red, :tone_green, :tone_blue].each do |key|
      assert_raise TypeError, "#{key}" do
        texture2.render_texture(texture, 0, 0, key => false)
      end
    end
    assert_raise TypeError do
      texture2.render_texture(Color.new(0, 0, 0, 0), 0, 0)
    end
  end
  
  def test_render_texture_blending
    src_texture = Texture.new(256, 1)
    src_texture.width.times do |i|
      src_texture[i, 0] = Color.new(127, 128, 129, i)
    end
    dst_texture = Texture.new(256, 1)
    dst_texture.render_texture(src_texture, 0, 0)
    src_texture.width.times do |i|
      assert_equal Color.new(127, 128, 129, i), dst_texture[i, 0]
    end
  end
  
  def test_render_texture_xy
    texture = Texture.load("images/ruby")
    texture2 = Texture.new(texture.width, texture.height)
    texture2.render_texture(texture, 10, 11)
    texture2.height.times do |j|
      texture2.width.times do |i|
        if i < 10 or j < 11
          assert_equal Color.new(0, 0, 0, 0), texture2[i, j]
        else
          p1 = texture[i - 10, j - 11]
          p2 = texture2[i, j]
          if p2.alpha != 0
            assert_equal p1, p2
          else
            assert_equal Color.new(p1.red, p1.green, p1.blue, 0), p2
          end
        end
      end
    end
    texture2.clear
    texture2.render_texture(texture, -12, -13)
    texture2.height.times do |j|
      texture2.width.times do |i|
        if i < texture2.width - 12 and j < texture2.height - 13
          p1 = texture[i + 12, j + 13]
          p2 = texture2[i, j]
          if p2.alpha != 0
            assert_equal p1, p2
          else
            assert_equal Color.new(p1.red, p1.green, p1.blue, 0), p2
          end
        else
          assert_equal Color.new(0, 0, 0, 0), texture2[i, j]
        end
      end
    end
  end
  
  def test_render_texture_scale
    texture = Texture.load("images/ruby")
    texture2 = Texture.new(texture.width, texture.height)
    texture2.render_texture(texture, 0, 0, :scale_x => 2, :scale_y => 2)
    texture2.height.times do |j|
      texture2.width.times do |i|
        p1 = texture[i / 2, j / 2]
        p2 = texture2[i, j]
        if p2.alpha != 0
          assert_equal p1, p2
        else
          assert_equal Color.new(p1.red, p1.green, p1.blue, 0), p2
        end
      end
    end
    texture2.clear
    texture2.render_texture(texture, 0, 0, :scale_x => 3, :scale_y => 4)
    texture2.height.times do |j|
      texture2.width.times do |i|
        p1 = texture[i / 3, j / 4]
        p2 = texture2[i, j]
        if p2.alpha != 0
          assert_equal p1, p2
        else
          assert_equal Color.new(p1.red, p1.green, p1.blue, 0), p2
        end
      end
    end
    w = texture.width
    h = texture.height
    texture2.clear
    texture2.render_texture(texture, w, h, :scale_x => -1, :scale_y => -1)
    texture2.height.times do |j|
      texture2.width.times do |i|
        p1 = texture[w - i - 1, h - j - 1]
        p2 = texture2[i, j]
        if p2.alpha != 0
          assert_equal p1, p2
        else
          assert_equal Color.new(p1.red, p1.green, p1.blue, 0), p2
        end
      end
    end
    texture2.clear
    texture2.render_texture(texture, w / 2, 0, :scale_x => -1)
    texture2.height.times do |j|
      texture2.width.times do |i|
        if i < w / 2
          p1 = texture[w / 2 - i - 1, j]
          p2 = texture2[i, j]
          if p2.alpha != 0
            assert_equal p1, p2
          else
            assert_equal Color.new(p1.red, p1.green, p1.blue, 0), p2
          end
        else
          assert_equal Color.new(0, 0, 0, 0), texture2[i, j]
        end
      end
    end
  end

  def test_render_texture_matrix
    texture = Texture.load("images/ruby")
    texture2 = Texture.new(texture.width, texture.height)
    texture3 = Texture.new(texture.width, texture.height)
    texture2.render_texture(texture, 0, 0, :matrix => [1, 0, 0, 1])
    texture3.render_texture(texture, 0, 0)
    texture.height.times do |j|
      texture.width.times do |i|
        assert_equal texture3[i, j], texture2[i, j]
      end
    end
    texture2.clear
    texture3.clear
    texture2.render_texture(texture, 0, 0, :matrix => [2, 0, 0, 1])
    texture3.render_texture(texture, 0, 0, :scale_x => 2)
    texture.height.times do |j|
      texture.width.times do |i|
        assert_equal texture3[i, j], texture2[i, j]
      end
    end
    texture2.clear
    texture3.clear
    texture2.render_texture(texture, 0, 0, :matrix => [2.5, 0, 0, 1.5])
    texture3.render_texture(texture, 0, 0, :scale_x => 2.5, :scale_y => 1.5)
    texture.height.times do |j|
      texture.width.times do |i|
        assert_equal texture3[i, j], texture2[i, j]
      end
    end
    texture2.clear
    texture3.clear
    a = 45.degrees
    texture2.render_texture(texture, 0, 0,
                            :matrix => [Math.cos(a), -Math.sin(a), Math.sin(a), Math.cos(a)])
    texture3.render_texture(texture, 0, 0, :angle => a)
    texture.height.times do |j|
      texture.width.times do |i|
        assert_equal texture3[i, j], texture2[i, j]
      end
    end
    texture2.clear
    texture3.clear
    a = 45.degrees
    texture2.render_texture(texture, 0, 0,
                            :matrix => [Math.cos(a), -Math.sin(a), Math.sin(a), Math.cos(a)],
                            :center_x => 10, :center_y => 10)
    texture3.render_texture(texture, 0, 0,
                            :angle => a, :center_x => 10, :center_y => 10)
    texture.height.times do |j|
      texture.width.times do |i|
        assert_equal texture3[i, j], texture2[i, j]
      end
    end
    texture2.clear
    texture3.clear
    texture2.render_texture(texture, 0, 0, :matrix => [1, 2, 3, 4])
    texture3.render_texture(texture, 0, 0, :matrix => [[1, 2], [3, 4]])
    texture.height.times do |j|
      texture.width.times do |i|
        assert_equal texture3[i, j], texture2[i, j]
      end
    end
    texture2.clear
    texture3.clear
    assert_raise ArgumentError do
      texture2.render_texture(texture, 0, 0, :matrix => [1, 2, 3])
    end
    assert_raise ArgumentError do
      texture2.render_texture(texture, 0, 0, :matrix => [1, 2, 3, 4, 5])
    end
    assert_raise TypeError do
      texture2.render_texture(texture, 0, 0, :matrix => [1, 2, 3, :foo])
    end
    assert_raise TypeError do
      texture2.render_texture(texture, 0, 0, :matrix => [[1, 2], :foo])
    end
    assert_raise ArgumentError do
      texture2.render_texture(texture, 0, 0, :matrix => [[1, 2, 3], [4, 5]])
    end
    assert_raise ArgumentError do
      texture2.render_texture(texture, 0, 0, :matrix => [[1, 2], [3, 4], [5, 6]])
    end
    assert_raise TypeError do
      texture2.render_texture(texture, 0, 0, :matrix => [[1, 2], [3, :foo]])
    end
  end
  
  def test_render_texture_src_rect
    texture = Texture.load("images/ruby")
    texture2 = Texture.new(texture.width, texture.height)
    texture2.render_texture(texture, 10, 11, :src_x => 12, :src_y => 13, :src_width => 14, :src_height => 15)
    texture2.height.times do |j|
      texture2.width.times do |i|
        if 10 <= i and 11 <= j and i < 24 and j < 26
          p1 = texture[i - 10 + 12, j - 11 + 13]
          p2 = texture2[i, j]
          assert_equal p1, p2
        else
          assert_equal Color.new(0, 0, 0, 0), texture2[i, j]
        end
      end
    end
    texture2.clear
    texture2.render_texture(texture, 0, 0,
                            :src_x => -100, :src_y => -100,
                            :src_width => texture.width + 200,
                            :src_height => texture.height + 200)
    texture.height.times do |j|
      texture.width.times do |i|
        assert_equal texture[i, j], texture2[i, j]
      end
    end
    texture2.clear
    texture2.render_texture(texture, 0, 0, :src_x => -10)
    texture.height.times do |j|
      texture.width.times do |i|
        assert_equal texture[i, j], texture2[i, j]
      end
    end
    texture2.clear
    texture2.render_texture(texture, 0, 0, :src_x => texture.width + 10)
    texture.height.times do |j|
      texture.width.times do |i|
        assert_equal Color.new(0, 0, 0, 0), texture2[i, j]
      end
    end
    texture2.clear
    texture2.render_texture(texture, 0, 0, :src_y => -10)
    texture.height.times do |j|
      texture.width.times do |i|
        assert_equal texture[i, j], texture2[i, j]
      end
    end
    texture2.clear
    texture2.render_texture(texture, 0, 0, :src_y => texture.height + 10)
    texture.height.times do |j|
      texture.width.times do |i|
        assert_equal Color.new(0, 0, 0, 0), texture2[i, j]
      end
    end
    texture2.render_texture(texture, 0, 0, :src_x => 10, :src_width => -10)
    texture.height.times do |j|
      texture.width.times do |i|
        assert_equal Color.new(0, 0, 0, 0), texture2[i, j]
      end
    end
    texture2.clear
    texture2.render_texture(texture, 0, 0, :src_x => 10, :src_width => texture.width - 5)
    texture.height.times do |j|
      texture.width.times do |i|
        if i < texture.width - 10
          assert_equal texture[i + 10, j], texture2[i, j]
        else
          assert_equal Color.new(0, 0, 0, 0), texture2[i, j]
        end
      end
    end
    texture2.clear
    texture2.render_texture(texture, 0, 0, :src_y => 10, :src_height => -10)
    texture.height.times do |j|
      texture.width.times do |i|
        assert_equal Color.new(0, 0, 0, 0), texture2[i, j]
      end
    end
    texture2.clear
    texture2.render_texture(texture, 0, 0, :src_y => 10, :src_height => texture.height - 5)
    texture.height.times do |j|
      texture.width.times do |i|
        if j < texture.height - 10
          assert_equal texture[i, j + 10], texture2[i, j]
        else
          assert_equal Color.new(0, 0, 0, 0), texture2[i, j]
        end
      end
    end
    texture2.clear
    texture2.render_texture(texture, 0, 0,
                            :src_x => -10, :src_y => 0,
                            :src_width => 10, :src_height => 10)
    texture.height.times do |j|
      texture.width.times do |i|
        assert_equal Color.new(0, 0, 0, 0), texture2[i, j]
      end
    end
    texture2.clear
    texture2.render_texture(texture, 0, 0,
                            :src_x => texture.width + 100, :src_y => texture.height + 100,
                            :src_width => 100, :src_height => 100)
    texture.height.times do |j|
      texture.width.times do |i|
        assert_equal Color.new(0, 0, 0, 0), texture2[i, j]
      end
    end
    texture2.clear
    texture2.render_texture(texture, texture2.width, 0,
                            :src_x => 10, :src_y => 10,
                            :src_width => texture.width - 10, :src_height => texture.height - 10)
    texture.height.times do |j|
      texture.width.times do |i|
        assert_equal Color.new(0, 0, 0, 0), texture2[i, j]
      end
    end
    texture2.clear
    texture2.render_texture(texture, 0, texture2.height,
                            :src_x => 10, :src_y => 10,
                            :src_width => texture.width - 10, :src_height => texture.height - 10)
    texture.height.times do |j|
      texture.width.times do |i|
        assert_equal Color.new(0, 0, 0, 0), texture2[i, j]
      end
    end
    texture2.clear
    texture2.render_texture(texture, -texture2.width, 0,
                            :src_x => 10, :src_y => 10,
                            :src_width => texture.width - 10, :src_height => texture.height - 10)
    texture.height.times do |j|
      texture.width.times do |i|
        assert_equal Color.new(0, 0, 0, 0), texture2[i, j]
      end
    end
    texture2.clear
    texture2.render_texture(texture, 0, -texture2.height,
                            :src_x => 10, :src_y => 10,
                            :src_width => texture.width - 10, :src_height => texture.height - 10)
    texture.height.times do |j|
      texture.width.times do |i|
        assert_equal Color.new(0, 0, 0, 0), texture2[i, j]
      end
    end
    texture2.clear
    texture2.render_texture(texture, -texture2.width, -texture2.height,
                            :src_x => 10, :src_y => 10,
                            :src_width => texture.width - 10, :src_height => texture.height - 10)
    texture.height.times do |j|
      texture.width.times do |i|
        assert_equal Color.new(0, 0, 0, 0), texture2[i, j]
      end
    end
    texture2.clear
    texture2.render_texture(texture, -texture2.width, texture2.height,
                            :src_x => 10, :src_y => 10,
                            :src_width => texture.width - 10, :src_height => texture.height - 10)
    texture.height.times do |j|
      texture.width.times do |i|
        assert_equal Color.new(0, 0, 0, 0), texture2[i, j]
      end
    end
    texture2.clear
    texture2.render_texture(texture, texture2.width, -texture2.height,
                            :src_x => 10, :src_y => 10,
                            :src_width => texture.width - 10, :src_height => texture.height - 10)
    texture.height.times do |j|
      texture.width.times do |i|
        assert_equal Color.new(0, 0, 0, 0), texture2[i, j]
      end
    end
    texture2.clear
    texture2.render_texture(texture, texture2.width, texture2.height,
                            :src_x => 10, :src_y => 10,
                            :src_width => texture.width - 10, :src_height => texture.height - 10)
    texture.height.times do |j|
      texture.width.times do |i|
        assert_equal Color.new(0, 0, 0, 0), texture2[i, j]
      end
    end
    texture2.clear
    texture2.render_texture(texture, -10000, -10000,
                            :src_x => 10, :src_y => 10,
                            :src_width => texture.width - 10, :src_height => texture.height - 10)
    texture.height.times do |j|
      texture.width.times do |i|
        assert_equal Color.new(0, 0, 0, 0), texture2[i, j]
      end
    end
  end

  def test_render_texture_alpha
    texture = Texture.load("images/ruby")
    texture2 = Texture.new(texture.width, texture.height)
    [64, 128, 192].each do |alpha|
      texture2.fill(Color.new(0, 0, 0, 1))
      texture2.render_texture(texture, 0, 0, :alpha => alpha)
      texture2.height.times do |j|
        texture2.width.times do |i|
          p1 = texture[i, j]
          p2 = texture2[i, j]
          a = p1.alpha * alpha.quo(255)
          assert_in_delta p1.red   * a / 255, p2.red,   2
          assert_in_delta p1.green * a / 255, p2.green, 2
          assert_in_delta p1.blue  * a / 255, p2.blue,  2
          assert_in_delta [p1.alpha * (alpha.quo(255)), 1].max, p2.alpha, 2
        end
      end
    end
  end

  def test_render_texture_alpha2
    texture = Texture.new(256, 1)
    texture2 = Texture.new(texture.width, texture.height)
    texture.undump(Array.new(256){|i| i.chr * 4}.join, "rgba")
    texture2.fill(Color.new(0, 0, 0, 128))
    alpha = 100
    texture2.render_texture(texture, 0, 0, :alpha => alpha)
    256.times do |i|
      p1 = texture[i, 0]
      p2 = texture2[i, 0]
      assert_in_delta p1.red   * alpha.quo(255) * i.quo(255), p2.red,   2
      assert_in_delta p1.green * alpha.quo(255) * i.quo(255), p2.green, 2
      assert_in_delta p1.blue  * alpha.quo(255) * i.quo(255), p2.blue,  2
      assert_in_delta [i * alpha.quo(255), 128].max, p2.alpha, 2
    end
    texture2.fill(Color.new(12, 34, 56, 0))
    alpha = 100
    texture2.render_texture(texture, 0, 0, :alpha => alpha)
    256.times do |i|
      p1 = texture[i, 0]
      p2 = texture2[i, 0]
      assert_equal p1.red,   p2.red
      assert_equal p1.green, p2.green
      assert_equal p1.blue,  p2.blue
      assert_in_delta i * alpha.quo(255), p2.alpha, 2
    end
  end
    
  def test_render_texture_blend_type
    texture = Texture.load("images/ruby")
    texture2 = Texture.new(texture.width, texture.height)
    # none
    texture2.fill(Color.new(100, 110, 120, 130))
    texture2.render_texture(texture, 0, 0, :blend_type => :none)
    texture2.height.times do |j|
      texture2.width.times do |i|
        assert_equal texture[i, j], texture2[i, j]
      end
    end
    texture2.fill(Color.new(100, 110, 120, 130))
    texture2.render_texture(texture, 0, 0, :blend_type => :none, :scale_y => 2)
    texture2.height.times do |j|
      texture2.width.times do |i|
        assert_equal texture[i, j / 2], texture2[i, j]
      end
    end
    # alpha
    texture2.fill(Color.new(100, 110, 120, 130))
    texture2.render_texture(texture, 0, 0, :blend_type => :alpha)
    texture2.height.times do |j|
      texture2.width.times do |i|
        p1 = texture[i, j]
        p2 = texture2[i, j]
        a = p1.alpha
        assert_in_delta (p1.red   * a + 100 * (255 - a)).quo(255), p2.red,   2
        assert_in_delta (p1.green * a + 110 * (255 - a)).quo(255), p2.green, 2
        assert_in_delta (p1.blue  * a + 120 * (255 - a)).quo(255), p2.blue,  2
        assert_in_delta [a, 130].max, p2.alpha, 2
      end
    end
    # add
    texture2.fill(Color.new(100, 110, 120, 130))
    texture2.render_texture(texture, 0, 0, :blend_type => :add, :alpha => 128)
    texture2.height.times do |j|
      texture2.width.times do |i|
        p1 = texture[i, j]
        p2 = texture2[i, j]
        a = p1.alpha * 128.quo(255)
        assert_in_delta [p1.red   * a.quo(255) + 100, 255].min, p2.red,   2
        assert_in_delta [p1.green * a.quo(255) + 110, 255].min, p2.green, 2
        assert_in_delta [p1.blue  * a.quo(255) + 120, 255].min, p2.blue,  2
        assert_in_delta [a, 130].max, p2.alpha, 2
      end
    end
    texture2.clear
    texture2.render_texture(texture, 0, 0, :blend_type => :add, :alpha => 128)
    texture2.height.times do |j|
      texture2.width.times do |i|
        p1 = texture[i, j]
        p2 = texture2[i, j]
        a = p1.alpha * 128.quo(255)
        assert_equal p1.red,   p2.red
        assert_equal p1.green, p2.green
        assert_equal p1.blue,  p2.blue
        assert_in_delta a, p2.alpha, 2
      end
    end
    # sub
    texture2.fill(Color.new(100, 110, 120, 130))
    texture2.render_texture(texture, 0, 0, :blend_type => :sub, :alpha => 128)
    texture2.height.times do |j|
      texture2.width.times do |i|
        p1 = texture[i, j]
        p2 = texture2[i, j]
        a = p1.alpha * 128.quo(255)
        assert_in_delta [-p1.red   * a.quo(255) + 100, 0].max, p2.red,   2
        assert_in_delta [-p1.green * a.quo(255) + 110, 0].max, p2.green, 2
        assert_in_delta [-p1.blue  * a.quo(255) + 120, 0].max, p2.blue,  2
        assert_in_delta [a, 130].max, p2.alpha, 2
      end
    end
    texture2.clear
    texture2.render_texture(texture, 0, 0, :blend_type => :sub, :alpha => 128)
    texture2.height.times do |j|
      texture2.width.times do |i|
        p1 = texture[i, j]
        p2 = texture2[i, j]
        assert_equal 0, p2.red
        assert_equal 0, p2.green
        assert_equal 0, p2.blue
        assert_in_delta p1.alpha * 128.quo(255), p2.alpha, 2
      end
    end
    # mask
    texture2.fill(Color.new(100, 110, 120, 130))
    texture2.render_texture(texture, 0, 0, :blend_type => :mask)
    texture2.height.times do |j|
      texture2.width.times do |i|
        p1 = texture[i, j]
        p2 = texture2[i, j]
        assert_equal 100, p2.red
        assert_equal 110, p2.green
        assert_equal 120, p2.blue
        assert_equal p1.red, p2.alpha
      end
    end
    texture2.fill(Color.new(100, 110, 120, 130))
    texture2.render_texture(texture, 0, 0, :blend_type => :mask, :alpha => 128)
    texture2.height.times do |j|
      texture2.width.times do |i|
        p1 = texture[i, j]
        p2 = texture2[i, j]
        assert_equal 100, p2.red
        assert_equal 110, p2.green
        assert_equal 120, p2.blue
        assert_equal p1.red, p2.alpha
      end
    end
    texture2.fill(Color.new(100, 110, 120, 130))
    texture2.render_texture(texture, 0, 0, :blend_type => :mask,
                            :alpha => 128, :tone_red => 255)
    texture2.height.times do |j|
      texture2.width.times do |i|
        p1 = texture[i, j]
        p2 = texture2[i, j]
        assert_equal 100, p2.red
        assert_equal 110, p2.green
        assert_equal 120, p2.blue
        assert_equal p1.red, p2.alpha
      end
    end
  end
  
  def test_render_texture_tone
    texture = Texture.load("images/ruby")
    texture2 = Texture.new(texture.width, texture.height)
    texture2.render_texture(texture, 0, 0, {
      :tone_red => 0, :tone_green => 0, :tone_blue => 0, :saturation => 255,
    })
    texture2.height.times do |j|
      texture2.width.times do |i|
        p1 = texture[i, j]
        p2 = texture2[i, j]
        if p2.alpha != 0
          assert_equal p1, p2
        else
          assert_equal Color.new(p1.red, p1.green, p1.blue, 0), p2
        end
      end
    end
    texture2.clear
    texture2.render_texture(texture, 0, 0, {
      :tone_red => 0, :tone_green => 0, :tone_blue => 0, :saturation => 128,
    })
    texture2.height.times do |j|
      texture2.width.times do |i|
        p1 = texture[i, j]
        p2 = texture2[i, j]
        gray = (6969 * p1.red + 23434 * p1.green + 2365 * p1.blue) / 32768
        assert_in_delta (gray + p1.red) / 2,   p2.red,   1
        assert_in_delta (gray + p1.green) / 2, p2.green, 1
        assert_in_delta (gray + p1.blue) / 2,  p2.blue,  1
        assert_equal p1.alpha, p2.alpha
      end
    end
    texture2.clear
    texture2.render_texture(texture, 0, 0, {
      :tone_red => 0, :tone_green => 0, :tone_blue => 0, :saturation => 0,
    })
    texture2.height.times do |j|
      texture2.width.times do |i|
        p1 = texture[i, j]
        p2 = texture2[i, j]
        assert(p2.red == p2.green && p2.green == p2.blue)
        gray = (6969 * p1.red + 23434 * p1.green + 2365 * p1.blue) / 32768
        assert_in_delta gray, p2.red,   1
        assert_in_delta gray, p2.green, 1
        assert_in_delta gray, p2.blue,  1
        assert_equal p1.alpha, p2.alpha
      end
    end
    texture2.clear
    texture2.render_texture(texture, 0, 0, {
      :tone_red => 64, :tone_green => 128, :tone_blue => 192,
    })
    texture2.height.times do |j|
      texture2.width.times do |i|
        p1 = texture[i, j]
        p2 = texture2[i, j]
        assert_in_delta (255 + p1.red * 3) / 4,  p2.red,   1
        assert_in_delta (255 + p1.green) / 2,    p2.green, 1
        assert_in_delta (255 * 3 + p1.blue) / 4, p2.blue,  1
        assert_equal p1.alpha, p2.alpha
      end
    end
    texture2.clear
    texture2.render_texture(texture, 0, 0, {
      :tone_red => -192, :tone_green => -128, :tone_blue => -64,
    })
    texture2.height.times do |j|
      texture2.width.times do |i|
        p1 = texture[i, j]
        p2 = texture2[i, j]
        assert_in_delta p1.red / 4,      p2.red,   1
        assert_in_delta p1.green / 2,    p2.green, 1
        assert_in_delta p1.blue * 3 / 4, p2.blue,  1
        assert_equal p1.alpha, p2.alpha
      end
    end
    texture2.clear
    assert_raise ArgumentError do
      texture2.render_texture(texture, 0, 0,
                              :tone_red => -256)
    end
    assert_raise ArgumentError do
      texture2.render_texture(texture, 0, 0,
                              :tone_red => 256)
    end
    assert_raise ArgumentError do
      texture2.render_texture(texture, 0, 0,
                              :tone_green => -256)
    end
    assert_raise ArgumentError do
      texture2.render_texture(texture, 0, 0,
                              :tone_green => 256)
    end
    assert_raise ArgumentError do
      texture2.render_texture(texture, 0, 0,
                              :tone_blue => -256)
    end
    assert_raise ArgumentError do
      texture2.render_texture(texture, 0, 0,
                              :tone_blue => 256)
    end
    assert_raise ArgumentError do
      texture2.render_texture(texture, 0, 0,
                              :saturation => -1)
    end
    assert_raise ArgumentError do
      texture2.render_texture(texture, 0, 0,
                              :saturation => 256)
    end
  end

  def test_render_texture_self
    texture = Texture.load("images/ruby")
    texture2 = Texture.new(texture.width, texture.height)
    texture2.render_texture(texture, 0, 0)
    texture2.render_texture(texture2, 10, 10)
    texture2.height.times do |j|
      texture2.width.times do |i|
        if i < 10 or j < 10
          p1 = texture[i, j]
          p2 = texture2[i, j]
          assert_equal p1, p2
        else
          src = texture[i - 10, j - 10]
          dst = texture[i, j]
          p2 = texture2[i, j]
          if 0 < p2.alpha
            a = src.alpha
            assert_in_delta (src.red   * a + dst.red   * (255 - a)).quo(255), p2.red,   2
            assert_in_delta (src.green * a + dst.green * (255 - a)).quo(255), p2.green, 2
            assert_in_delta (src.blue  * a + dst.blue  * (255 - a)).quo(255), p2.blue,  2
            assert_equal [a, dst.alpha].max, p2.alpha
          else
            assert_equal Color.new(src.red, srcgreen, src.blue, [src.alpha, dst.alpha].max), p2
          end
        end
      end
    end
  end

end
