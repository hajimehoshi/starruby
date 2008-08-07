#!/usr/bin/env ruby

require "starruby"
include StarRuby

require "frozen_error"

class TestTexturePalette < Test::Unit::TestCase

  def test_palette
    texture = Texture.load("images/ruby")
    assert_nil texture.palette
    texture = Texture.load("images/ruby8")
    assert_nil texture.palette
    texture = Texture.load("images/ruby8", :palette => false)
    assert_nil texture.palette
    texture = Texture.load("images/ruby8", :palette => true)
    assert_kind_of Array, texture.palette
    assert texture.palette.frozen?
    assert_equal 255, texture.palette.size
    assert_equal 0, texture.palette[0].alpha
    assert_equal Color.new(0x79, 0x03, 0x00, 0xff), texture.palette[1]
    assert_equal Color.new(0x82, 0x00, 0x00, 0xff), texture.palette[2]
    assert_equal Color.new(0xff, 0xfc, 0xfb, 0xff), texture.palette[253]
    assert_equal Color.new(0xfd, 0xff, 0xfc, 0xff), texture.palette[254]
    texture = Texture.load("images/ruby8_without_alpha", :palette => true)
    assert_equal 253, texture.palette.size
    assert_equal Color.new(0x82, 0x00, 0x00, 0xff), texture.palette[0]
    assert_equal Color.new(0x73, 0x09, 0x03, 0xff), texture.palette[1]
    assert_equal Color.new(0xff, 0xfc, 0xfb, 0xff), texture.palette[251]
    assert_equal Color.new(0xfd, 0xff, 0xfc, 0xff), texture.palette[252]
    texture = Texture.load("images/ruby8_16colors", :palette => true)
    assert_equal 16, texture.palette.size
    assert_equal 0, texture.palette[0].alpha
    assert_equal Color.new(0x8c, 0x0c, 0x02, 0xff), texture.palette[1]
    assert_equal Color.new(0x96, 0x0f, 0x00, 0xff), texture.palette[2]
    assert_equal Color.new(0xfb, 0xf6, 0xf3, 0xff), texture.palette[14]
    assert_equal Color.new(0x7d, 0x0b, 0x06, 0xff), texture.palette[15]
  end

  def test_palette_frozen
    texture = Texture.load("images/ruby8", :palette => true)
    texture.freeze
    assert_kind_of Array, texture.palette
  end

  def test_palette_disposed
    texture = Texture.load("images/ruby8", :palette => true)
    texture.dispose
    assert_raise RuntimeError do
      texture.palette
    end
  end

  def test_palette_dup
    texture = Texture.load("images/ruby8", :palette => true)
    assert_equal texture.palette, texture.dup.palette
    assert_equal texture.palette, texture.clone.palette
  end

  def test_palette2
    orig_normal_texture  = normal_texture  = Texture.load("images/ruby8")
    orig_palette_texture = palette_texture = Texture.load("images/ruby8", :palette => true)
    target_texture = Texture.load("images/ruby")
    if Font.exist?("Arial")
      font = Font.new("Arial", 16)
    elsif Font.exist?("FreeSans")
      font = Font.new("FreeSans", 16)
    elsif Font.exist?("Helvetica Neue")
      font = Font.new("Helvetica Neue", 16)
    else
      flunk
    end
    # []=
    normal_texture[0, 0] = Color.new(0, 0, 0, 0)
    assert_raise StarRubyError do
      palette_texture[0, 0] = Color.new(0, 0, 0, 0)
    end
    # change_hue
    normal_texture.change_hue(0)
    palette_texture.change_hue(0)
    # change_hue!
    normal_texture.change_hue!(0)
    palette_texture.change_hue!(0)
    # clear
    normal_texture  = orig_normal_texture.dup
    palette_texture = orig_palette_texture.dup
    normal_texture.clear
    assert_raise StarRubyError do
      palette_texture.clear
    end
    # fill
    normal_texture  = orig_normal_texture.dup
    palette_texture = orig_palette_texture.dup
    normal_texture.fill(Color.new(1, 2, 3, 4))
    assert_raise StarRubyError do
      palette_texture.fill(Color.new(1, 2, 3, 4))
    end
    # fill_rect
    normal_texture  = orig_normal_texture.dup
    palette_texture = orig_palette_texture.dup
    normal_texture.fill_rect(1, 2, 3, 4, Color.new(1, 2, 3, 4))
    assert_raise StarRubyError do
      palette_texture.fill_rect(1, 2, 3, 4, Color.new(1, 2, 3, 4))
    end
    # render_in_perspective
    normal_texture  = orig_normal_texture.dup
    palette_texture = orig_palette_texture.dup
    normal_texture.render_in_perspective(target_texture)
    assert_raise StarRubyError do
      palette_texture.render_in_perspective(target_texture)
    end
    # render_line
    normal_texture  = orig_normal_texture.dup
    palette_texture = orig_palette_texture.dup
    normal_texture.render_line(1, 2, 3, 4, Color.new(1, 2, 3, 4))
    assert_raise StarRubyError do
      palette_texture.render_line(1, 2, 3, 4, Color.new(1, 2, 3, 4))
    end
    # render_pixel
    normal_texture  = orig_normal_texture.dup
    palette_texture = orig_palette_texture.dup
    normal_texture.render_pixel(1, 2, Color.new(1, 2, 3, 4))
    assert_raise StarRubyError do
      palette_texture.render_pixel(1, 2, Color.new(1, 2, 3, 4))
    end
    # render_rect
    normal_texture  = orig_normal_texture.dup
    palette_texture = orig_palette_texture.dup
    normal_texture.render_rect(1, 2, 3, 4, Color.new(1, 2, 3, 4))
    assert_raise StarRubyError do
      palette_texture.render_rect(1, 2, 3, 4, Color.new(1, 2, 3, 4))
    end
    # render_text
    normal_texture  = orig_normal_texture.dup
    palette_texture = orig_palette_texture.dup
    normal_texture.render_text("text", 0, 0, font, Color.new(1, 2, 3, 4))
    assert_raise StarRubyError do
      palette_texture.render_text("text", 0, 0, font, Color.new(1, 2, 3, 4))
    end
    # render_texture
    normal_texture  = orig_normal_texture.dup
    palette_texture = orig_palette_texture.dup
    normal_texture.render_texture(target_texture, 0, 0)
    assert_raise StarRubyError do
      palette_texture.render_texture(target_texture, 0, 0)
    end
    # undump
    normal_texture  = orig_normal_texture.dup
    palette_texture = orig_palette_texture.dup
    size = normal_texture.width * normal_texture.height
    normal_texture.undump("\0" * size, "r")
    assert_raise StarRubyError do
      palette_texture.undump("\0" * size, "r")
    end
  end
  
  def test_change_palette_original_palette
    texture = Texture.load("images/ruby8", :palette => true)
    orig_texture = texture.dup
    texture.change_palette!(texture.palette)
    texture.height.times do |j|
      texture.width.times do |i|
        assert_equal orig_texture[i, j], texture[i, j], [i, j].inspect
      end
    end
    texture = Texture.load("images/ruby8", :palette => true)
    texture2 = texture.change_palette(texture.palette)
    texture.height.times do |j|
      texture.width.times do |i|
        assert_equal texture[i, j], texture2[i, j], [i, j].inspect
      end
    end
  end

  def test_change_palette_null_palette
    texture = Texture.load("images/ruby8", :palette => true)
    size = texture.palette.size
    texture.change_palette!([])
    assert_equal ([Color.new(0, 0, 0, 0)] * size), texture.palette
    texture.height.times do |j|
      texture.width.times do |i|
        assert_equal Color.new(0, 0, 0, 0), texture[i, j]
      end
    end
    texture = Texture.load("images/ruby8", :palette => true)
    orig_texture = texture.dup
    texture2 = texture.change_palette([])
    assert_equal orig_texture.palette, texture.palette
    assert_equal ([Color.new(0, 0, 0, 0)] * size), texture2.palette
    texture.height.times do |j|
      texture.width.times do |i|
        assert_equal orig_texture[i, j], texture[i, j]
        assert_equal Color.new(0, 0, 0, 0), texture2[i, j]
      end
    end
  end

  def test_change_palette_small_palette
    texture = Texture.load("images/ruby8", :palette => true)
    size = texture.palette.size
    assert_equal Color.new(0x79, 0x03, 0x00, 0xff), texture.palette[1]
    assert_equal Color.new(0x79, 0x03, 0x00, 0xff), texture[93, 8]
    assert_equal Color.new(0x82, 0x00, 0x00, 0xff), texture.palette[2]
    assert_equal Color.new(0x82, 0x00, 0x00, 0xff), texture[81, 55]
    texture.change_palette!([Color.new(1, 2, 3, 4),
                             Color.new(5, 6, 7, 8),
                             Color.new(9, 10, 11, 12)])
    assert size, texture.palette.size
    assert_equal Color.new(1, 2, 3, 4), texture.palette[0]
    assert_equal Color.new(5, 6, 7, 8), texture.palette[1]
    assert_equal Color.new(9, 10, 11, 12), texture.palette[2]
    assert_equal [Color.new(0, 0, 0, 0)] * (size - 3), texture.palette[3, size - 3]
    assert_equal Color.new(1, 2, 3, 4), texture[0, 0]
    assert_equal Color.new(5, 6, 7, 8), texture[93, 8]
    assert_equal Color.new(9, 10, 11, 12), texture[81, 55]
    texture = Texture.load("images/ruby8", :palette => true)
    orig_texture = texture.dup
    texture2 = texture.change_palette([Color.new(1, 2, 3, 4),
                                       Color.new(5, 6, 7, 8),
                                       Color.new(9, 10, 11, 12)])
    assert_equal orig_texture.palette, texture.palette
    texture.height.times do |j|
      texture.width.times do |i|
        assert_equal orig_texture[i, j], texture[i, j]
      end
    end
    assert_equal Color.new(1, 2, 3, 4), texture2.palette[0]
    assert_equal Color.new(5, 6, 7, 8), texture2.palette[1]
    assert_equal Color.new(9, 10, 11, 12), texture2.palette[2]
    assert_equal [Color.new(0, 0, 0, 0)] * (size - 3), texture2.palette[3, size - 3]
    assert_equal Color.new(1, 2, 3, 4), texture2[0, 0]
    assert_equal Color.new(5, 6, 7, 8), texture2[93, 8]
    assert_equal Color.new(9, 10, 11, 12), texture2[81, 55]
  end

  def test_change_palette_big_palette
    texture = Texture.load("images/ruby8", :palette => true)
    size = texture.palette.size
    texture.change_palette!([Color.new(0x24, 0x3f, 0x6a, 0x88)] * 512)
    assert_equal size, texture.palette.size
    texture.height.times do |j|
      texture.width.times do |i|
        assert_equal Color.new(0x24, 0x3f, 0x6a, 0x88), texture[i, j]
      end
    end
    texture = Texture.load("images/ruby8", :palette => true)
    orig_texture = texture.dup
    texture2 = texture.change_palette([Color.new(0x24, 0x3f, 0x6a, 0x88)] * 512)
    assert_equal orig_texture.palette, texture.palette
    texture.height.times do |j|
      texture.width.times do |i|
        assert_equal orig_texture[i, j], texture[i, j]
        assert_equal Color.new(0x24, 0x3f, 0x6a, 0x88), texture2[i, j]
      end
    end
  end

  def test_change_palette_without_palette
    texture = Texture.load("images/ruby", :palette => true)
    assert_raise StarRubyError do
      texture.change_palette!([])
    end
    assert_raise StarRubyError do
      texture.change_palette([])
    end
  end

  def test_change_palette_frozen
    texture = Texture.load("images/ruby8", :palette => true)
    texture.freeze
    assert_raise FrozenError do
      texture.change_palette!([])
    end
    texture.change_palette([])
  end

  def test_change_palette_disposed
    texture = Texture.load("images/ruby8", :palette => true)
    texture.dispose
    assert_raise RuntimeError do
      texture.change_palette!([])
    end
    assert_raise RuntimeError do
      texture.change_palette([])
    end
  end

  def test_change_palette_type
    texture = Texture.load("images/ruby8", :palette => true)
    assert_raise TypeError do
      texture.change_palette!(false)
    end
    assert_raise TypeError do
      texture.change_palette(false)
    end
  end

  def test_change_hue_palette
    texture = Texture.load("images/ruby8", :palette => true)
    orig_texture = texture.clone
    # 0
    texture = orig_texture.change_hue(0)
    texture.height.times do |j|
      texture.width.times do |i|
        assert_equal orig_texture[i, j], texture[i, j]
      end
    end
    assert_equal orig_texture.palette, texture.palette
    texture = orig_texture.clone
    texture.change_hue!(0)
    texture.height.times do |j|
      texture.width.times do |i|
        assert_equal orig_texture[i, j], texture[i, j]
      end
    end
    assert_equal orig_texture.palette, texture.palette
    # (2/3) * pi
    texture = orig_texture.change_hue(Math::PI * 2 / 3)
    texture.height.times do |j|
      texture.width.times do |i|
        p1 = orig_texture[i, j]
        p2 = texture[i, j]
        assert_in_delta p1.blue,  p2.red,   1
        assert_in_delta p1.red,   p2.green, 1
        assert_in_delta p1.green, p2.blue,  1
        assert_equal p1.alpha, p2.alpha
      end
    end
    orig_texture.palette.zip(texture.palette).each_with_index do |ps, i|
      p1, p2 = ps
      assert_in_delta p1.blue,  p2.red,   1
      assert_in_delta p1.red,   p2.green, 1
      assert_in_delta p1.green, p2.blue,  1
      assert_equal p1.alpha, p2.alpha
    end
    texture = orig_texture.clone
    texture.change_hue!(Math::PI * 2 / 3)
    texture.height.times do |j|
      texture.width.times do |i|
        p1 = orig_texture[i, j]
        p2 = texture[i, j]
        assert_in_delta p1.blue,  p2.red,   1
        assert_in_delta p1.red,   p2.green, 1
        assert_in_delta p1.green, p2.blue,  1
        assert_equal p1.alpha, p2.alpha
      end
    end
    orig_texture.palette.zip(texture.palette).each_with_index do |ps, i|
      p1, p2 = ps
      assert_in_delta p1.blue,  p2.red,   1
      assert_in_delta p1.red,   p2.green, 1
      assert_in_delta p1.green, p2.blue,  1
      assert_equal p1.alpha, p2.alpha
    end
    # (4/3) * pi
    texture = orig_texture.change_hue(Math::PI * 4 / 3)
    texture.height.times do |j|
      texture.width.times do |i|
        p1 = orig_texture[i, j]
        p2 = texture[i, j]
        assert_in_delta p1.green, p2.red,   1
        assert_in_delta p1.blue,  p2.green, 1
        assert_in_delta p1.red,   p2.blue,  1
        assert_equal p1.alpha, p2.alpha
      end
    end
    orig_texture.palette.zip(texture.palette).each_with_index do |ps, i|
      p1, p2 = ps
      assert_in_delta p1.green, p2.red,   1
      assert_in_delta p1.blue,  p2.green, 1
      assert_in_delta p1.red,   p2.blue,  1
      assert_equal p1.alpha, p2.alpha
    end
    texture = orig_texture.clone
    texture.change_hue!(Math::PI * 4 / 3)
    texture.height.times do |j|
      texture.width.times do |i|
        p1 = orig_texture[i, j]
        p2 = texture[i, j]
        assert_in_delta p1.green, p2.red,   1
        assert_in_delta p1.blue,  p2.green, 1
        assert_in_delta p1.red,   p2.blue,  1
        assert_equal p1.alpha, p2.alpha
      end
    end
    orig_texture.palette.zip(texture.palette).each_with_index do |ps, i|
      p1, p2 = ps
      assert_in_delta p1.green, p2.red,   1
      assert_in_delta p1.blue,  p2.green, 1
      assert_in_delta p1.red,   p2.blue,  1
      assert_equal p1.alpha, p2.alpha
    end
  end

  def test_change_palette_overrided_dup
    texture = Texture.load("images/ruby8", :palette => true)
    palette = texture.palette.map do
      Color.new(rand(0xff), rand(0xff), rand(0xff), rand(0xff))
    end
    texture2 = texture.change_palette(palette)
    def texture.dup
      Color.new(0, 0, 0, 0)
    end
    texture3 = texture.change_palette(palette)
    assert_equal texture2.width, texture3.width
    assert_equal texture2.height, texture3.height
    texture2.height.times do |j|
      texture2.width.times do |i|
        assert_equal texture2[i, j], texture3[i, j]
      end
    end
  end

  def test_change_palette_overrided_clone
    texture = Texture.load("images/ruby8", :palette => true)
    palette = texture.palette.map do
      Color.new(rand(0xff), rand(0xff), rand(0xff), rand(0xff))
    end
    texture2 = texture.change_palette(palette)
    def texture.clone
      Color.new(0, 0, 0, 0)
    end
    texture3 = texture.change_palette(palette)
    assert_equal texture2.width, texture3.width
    assert_equal texture2.height, texture3.height
    texture2.height.times do |j|
      texture2.width.times do |i|
        assert_equal texture2[i, j], texture3[i, j]
      end
    end
  end

end
