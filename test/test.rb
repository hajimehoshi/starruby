#!/usr/bin/env ruby

require "../starruby"
require "test/unit"

include StarRuby

class ColorTest < Test::Unit::TestCase

  def test_color
    c1 = Color.new(1, 2, 3, 4)
    c2 = Color.new(5, 6, 7)

    assert_equal 1, c1.red
    assert_equal 2, c1.green
    assert_equal 3, c1.blue
    assert_equal 4, c1.alpha
    assert_equal 255, c2.alpha

    assert c1 == Color.new(1, 2, 3, 4)
    assert c2 == Color.new(5, 6, 7)
    assert c2 == Color.new(5, 6, 7, 255)
    assert c1 != Color.new(1, 2, 3, 5)
    assert c1 != Object.new
    assert c1.eql?(Color.new(1, 2, 3, 4))
    assert_equal c1.hash, Color.new(1, 2, 3, 4).hash
    assert_equal c2.hash, Color.new(5, 6, 7).hash
  end

  def test_color_overflow
    c = Color.new(-100, 256, -1, 999)
    assert_equal 0,   c.red
    assert_equal 255, c.green
    assert_equal 0,   c.blue
    assert_equal 255, c.alpha
    
    c = Color.new(999, -1, 256, -100)
    assert_equal 255, c.red
    assert_equal 0,   c.green
    assert_equal 255, c.blue
    assert_equal 0,   c.alpha
  end
  
  def test_to_s
    c = Color.new(1, 2, 3, 4)
    assert_equal "#<StarRuby::Color alpha=4, red=1, green=2, blue=3>", c.to_s
    c = Color.new(255, 255, 255, 255)
    assert_equal "#<StarRuby::Color alpha=255, red=255, green=255, blue=255>", c.to_s
  end
  
end

class FontTest < Test::Unit::TestCase
  
  CSIDL_FONTS = 0x0014
  
  def test_load_path
    assert_equal ".", Font.load_path[0]
    case RUBY_PLATFORM
    when /mswin32|cygwin|mingw32|bccwin32|interix|djgpp/
      require "Win32API"
      path = " " * 256
      p = Win32API.new("SHFOLDER", "SHGetFolderPath", %w(p i p i p), "i")
      p.call(0, CSIDL_FONTS, 0, 0, path)
      assert_equal path.strip.delete("\0"), Font.load_path[1]
    end
  end
  
  def test_exist
    case RUBY_PLATFORM
    when /mswin32|cygwin|mingw32|bccwin32|interix|djgpp/
      assert_equal true,  Font.exist?("arial")
      assert_equal true,  Font.exist?("arial.ttf")
      assert_equal false, Font.exist?("arial.ttc")
      assert_equal true,  Font.exist?("msgothic")
      assert_equal false, Font.exist?("msgothic.ttf")
      assert_equal true,  Font.exist?("msgothic.ttc")
      assert_equal false, Font.exist?("NotFont")
      assert_equal false, Font.exist?("NotFont.ttf")
      assert_equal false, Font.exist?("NotFont.ttc")
    end
    #assert_equal true,  Font.exist?("Resources/Fonts/04B_11__")
    #assert_equal true,  Font.exist?("Resources/Fonts/04B_11__.TTF")
    #assert_equal false, Font.exist?("Resources/Fonts/04B_12__")
    #assert_equal false, Font.exist?("Hoge/Resources/Fonts/04B_11__")
    # ファイルの中身は精査しない
    #assert_equal true,  Font.exist?("Resources/Images/Ruby.png")
  end
  
end

class GameTest < Test::Unit::TestCase
  
  def test_game
    assert_equal false, Game.running?
    assert_equal "hoge1", (Game.title = "hoge1")
    assert_equal "hoge1", Game.title
    assert_equal 30, Game.fps;
    assert_equal 31, (Game.fps = 31)
    assert_equal 31, Game.fps
    Game.run do
      assert_equal true, Game.running?
      assert_equal "hoge2", (Game.title = "hoge2")
      assert_equal "hoge2", Game.title
      assert_equal 32, (Game.fps = 32)
      assert_equal 32, Game.fps
      assert_raise StarRubyError do
        Game.run {}
      end
      Game.terminate
      assert_equal true, Game.running?
    end
    assert_equal false, Game.running?
    assert_equal "hoge3", (Game.title = "hoge3")
    assert_equal "hoge3", Game.title
    assert_equal 33, (Game.fps = 33)
    assert_equal 33, Game.fps
  end
  
end

class ScreenTest < Test::Unit::TestCase
  
  def test_size
    assert_equal 320, Screen.width
    assert_equal 240, Screen.height
    assert_equal [320, 240], Screen.size
    assert_equal true, Screen.size.frozen?
  end
  
  def test_offscreen
    assert_kind_of Texture, Screen.offscreen
    assert_equal Screen.width, Screen.offscreen.width
    assert_equal Screen.height, Screen.offscreen.height
  end
  
end

class TextureTest < Test::Unit::TestCase
  
  def test_new
    texture = Texture.new(123, 456)
    assert_equal 123, texture.width
    assert_equal 456, texture.height
    assert_equal [123, 456], texture.size
    assert_equal true, texture.size.frozen?
  end
  
  def test_load
    texture = Texture.load("images/ruby.png")
    assert_equal 50, texture.width
    assert_equal 35, texture.height
    assert_equal [50, 35], texture.size
    
    assert_raise Errno::ENOENT do
      Texture.load("invalid_path/foo.png")
    end
    
    assert_raise Errno::ENOENT do
      Texture.load("images/not_existed.png")
    end
    
    assert_not_nil Texture.load("images/ruby");
    assert_not_nil Texture.load("images/rubypng");
    assert_not_nil Texture.load("images/ruby.foo");
    assert_not_nil Texture.load("images/ruby.foo.png");
    
    assert_raise StarRubyError do
      Texture.load("images/not_image");
    end
    
    assert_raise StarRubyError do
      Texture.load("images/not_image.txt");
    end
  end
  
  def test_clone
    texture = Texture.load("images/ruby")
    texture2 = texture.clone
    assert_equal texture.size, texture2.size
    texture.freeze
    assert texture.clone.frozen?
    texture.height.times do |j|
      texture.width.times do |i|
        assert_equal texture.get_pixel(i, j), texture2.get_pixel(i, j)
      end
    end
  end
  
  def test_dup
    texture = Texture.load("images/ruby")
    texture2 = texture.dup
    assert_equal texture.size, texture2.size
    texture.freeze
    assert !texture.dup.frozen?
    texture.height.times do |j|
      texture.width.times do |i|
        assert_equal texture.get_pixel(i, j), texture2.get_pixel(i, j)
      end
    end
  end
  
  def test_dispose
    texture = Texture.load("images/ruby")
    assert_equal false, texture.disposed?
    texture.dispose
    assert_equal true, texture.disposed?
    assert_raise StarRubyError do
      texture.dispose
    end
    # TODO: change_hue
  end
  
  def test_get_and_set_pixel
    texture = Texture.new(3, 3)
    texture.height.times do |y|
      texture.width.times do |x|
        assert_equal Color.new(0, 0, 0, 0), texture.get_pixel(x,y)
      end
    end
    
    begin
      texture.get_pixel(-1, 2)
      flunk
    rescue ArgumentError => e
      assert_equal "index out of range: (-1, 2)", e.message
    end
    
    begin
      texture.get_pixel(2, -1)
      flunk
    rescue ArgumentError => e
      assert_equal "index out of range: (2, -1)", e.message
    end
    
    begin
      texture.get_pixel(3, 2)
      flunk
    rescue ArgumentError => e
      assert_equal "index out of range: (3, 2)", e.message
    end
    
    begin
      texture.get_pixel(2, 3)
      flunk
    rescue ArgumentError => e
      assert_equal "index out of range: (2, 3)", e.message
    end
    
    assert_equal Color.new(31, 41, 59, 26), texture.set_pixel(0, 1, Color.new(31, 41, 59, 26))
    assert_equal Color.new(53, 58, 97, 92), texture.set_pixel(1, 2, Color.new(53, 58, 97, 92))
    assert_equal Color.new(65, 35, 89, 79), texture.set_pixel(2, 0, Color.new(65, 35, 89, 79))
    assert_equal Color.new(31, 41, 59, 26), texture.get_pixel(0, 1);
    assert_equal Color.new(53, 58, 97, 92), texture.get_pixel(1, 2);
    assert_equal Color.new(65, 35, 89, 79), texture.get_pixel(2, 0);
    
    begin
      texture.set_pixel(-1, 2, Color.new(0, 0, 0))
      flunk
    rescue ArgumentError => e
      assert_equal "index out of range: (-1, 2)", e.message
    end
    
    begin
      texture.set_pixel(2, -1, Color.new(0, 0, 0))
      flunk
    rescue ArgumentError => e
      assert_equal "index out of range: (2, -1)", e.message
    end
    
    begin
      texture.set_pixel(3, 2, Color.new(0, 0, 0))
      flunk
    rescue ArgumentError => e
      assert_equal "index out of range: (3, 2)", e.message
    end
    
    begin
      texture.set_pixel(2, 3, Color.new(0, 0, 0))
      flunk
    rescue ArgumentError => e
      assert_equal "index out of range: (2, 3)", e.message
    end
  end
  
  def test_get_pixel_disposed
    texture = Texture.new(3, 3)
    texture.dispose
    assert_raise TypeError do
      texture.get_pixel(0, 1)
    end
  end
  
  def test_set_pixel_frozen
    texture = Texture.new(3, 3)
    texture.freeze
    assert_raise TypeError do
      texture.set_pixel(0, 1, Color.new(31, 41, 59, 26))
    end
  end
  
  def test_set_pixel_disposed
    texture = Texture.new(3, 3)
    texture.dispose
    assert_raise TypeError do
      texture.set_pixel(0, 1, Color.new(31, 41, 59, 26))
    end
  end
  
  def test_clear
    texture = Texture.load("images/ruby")
    texture.clear
    texture.height.times do |y|
      texture.width.times do |x|
        assert_equal Color.new(0, 0, 0, 0), texture.get_pixel(x, y)
      end
    end
  end
  
  def test_clear_frozen
    texture = Texture.load("images/ruby")
    texture.freeze
    assert_raise TypeError do
      texture.clear
    end
  end
  
  def test_clear_disposed
    texture = Texture.load("images/ruby")
    texture.dispose
    assert_raise TypeError do
      texture.clear
    end
  end
  
  def test_fill
    texture = Texture.load("images/ruby")
    texture.fill(Color.new(31, 41, 59, 26))
    texture.height.times do |y|
      texture.width.times do |x|
        assert_equal Color.new(31, 41, 59, 26), texture.get_pixel(x, y)
      end
    end
  end
  
  def test_fill_frozen
    texture = Texture.load("images/ruby")
    texture.freeze
    assert_raise TypeError do
      texture.fill(Color.new(31, 41, 59, 26))
    end
  end
  
  def test_fill_disposed
    texture = Texture.load("images/ruby")
    texture.dispose
    assert_raise TypeError do
      texture.fill(Color.new(31, 41, 59, 26))
    end
  end

  def test_fill_rect
    texture = Texture.load("images/ruby")
    orig_texture = texture.clone
    texture.fill_rect 10, 11, 12, 13, Color.new(12, 34, 56, 78)
    texture.height.times do |y|
      texture.width.times do |x|
        if 10 <= x and 11 <= y and x < 22 and y < 24
          assert_equal Color.new(12, 34, 56, 78), texture.get_pixel(x, y)
        else
          assert_equal orig_texture.get_pixel(x, y), texture.get_pixel(x, y)
        end
      end
    end
  end
  
  def test_fill_rect_frozen
    texture = Texture.load("images/ruby")
    texture.freeze
    assert_raise TypeError do
      texture.fill_rect 10, 11, 12, 13, Color.new(12, 34, 56, 78)
    end
  end
  
  def test_fill_rect_disposed
    texture = Texture.load("images/ruby")
    texture.dispose
    assert_raise TypeError do
      texture.fill_rect 10, 11, 12, 13, Color.new(12, 34, 56, 78)
    end
  end
  
  def test_change_hue
    texture = Texture.load("images/ruby")
    orig_texture = texture.clone
    texture.change_hue(0)
    texture.height.times do |y|
      texture.width.times do |x|
        assert_equal orig_texture.get_pixel(x, y), texture.get_pixel(x, y)
      end
    end
    texture = orig_texture.clone
    texture.change_hue(Math::PI * 2 / 3)
    texture.height.times do |y|
      texture.width.times do |x|
        p1 = orig_texture.get_pixel(x, y)
        p2 = texture.get_pixel(x, y)
        assert_in_delta p1.blue,  p2.red,   1
        assert_in_delta p1.red,   p2.green, 1
        assert_in_delta p1.green, p2.blue,  1
        assert_equal p1.alpha, p2.alpha
      end
    end
    texture = orig_texture.clone
    texture.change_hue(Math::PI * 4 / 3)
    texture.height.times do |y|
      texture.width.times do |x|
        p1 = orig_texture.get_pixel(x, y)
        p2 = texture.get_pixel(x, y)
        assert_in_delta p1.green, p2.red,   1
        assert_in_delta p1.blue,  p2.green, 1
        assert_in_delta p1.red,   p2.blue,  1
        assert_equal p1.alpha, p2.alpha
      end
    end
  end
  
  def test_change_hue_frozen
    texture = Texture.load("images/ruby")
    texture.freeze
    assert_raise TypeError do
      texture.change_hue(Math::PI)
    end
  end
  
  def test_change_hue_disposed
    texture = Texture.load("images/ruby")
    texture.dispose
    assert_raise TypeError do
      texture.change_hue(Math::PI)
    end
  end
  
  def test_render_texture
    texture = Texture.load("images/ruby")
    texture2 = Texture.new(texture.width, texture.height)
    texture2.render_texture(texture, 0, 0)
    texture.height.times do |y|
      texture.width.times do |x|
        assert_equal texture.get_pixel(x, y), texture2.get_pixel(x, y)
      end
    end
    texture2.fill(Color.new(128, 128, 128, 128))
    texture2.render_texture(texture, 0, 0)
    texture.height.times do |y|
      texture.width.times do |x|
        p1 = texture.get_pixel(x, y)
        p2 = texture2.get_pixel(x, y)
        case p1.alpha
        when 255
          assert_equal p1.red,   p2.red
          assert_equal p1.green, p2.green
          assert_equal p1.blue,  p2.blue
        when 0
          assert_equal 128, p2.red
          assert_equal 128, p2.green
          assert_equal 128, p2.blue
        else
          flunk
        end
        assert_equal [128, p1.alpha].max, p2.alpha
      end
    end
  end
  
  def test_render_texture_frozen
    texture = Texture.load("images/ruby")
    texture2 = Texture.new(texture.width, texture.height)
    texture2.freeze
    assert_raise TypeError do
      texture2.render_texture(texture, 0, 0)
    end
  end
  
  def test_render_texture_disposed
    texture = Texture.load("images/ruby")
    texture2 = Texture.new(texture.width, texture.height)
    texture2.dispose
    assert_raise TypeError do
      texture2.render_texture(texture, 0, 0)
    end
    texture3 = Texture.new(texture.width, texture.height)
    texture.dispose
    assert_raise TypeError do
      texture3.render_texture(texture, 0, 0)
    end
  end
  
  def test_render_texture_xy
    texture = Texture.load("images/ruby")
    texture2 = Texture.new(texture.width, texture.height)
    texture2.render_texture(texture, 10, 11)
    texture2.height.times do |y|
      texture2.width.times do |x|
        if x < 10 or y < 11
          assert_equal Color.new(0, 0, 0, 0), texture2.get_pixel(x, y)
        else
          assert_equal texture.get_pixel(x-10, y-11), texture2.get_pixel(x, y)
        end
      end
    end
    texture2.clear
    texture2.render_texture(texture, -12, -13)
    texture2.height.times do |y|
      texture2.width.times do |x|
        if x < texture2.width - 12 and y < texture2.height - 13
          assert_equal texture.get_pixel(x+12, y+13), texture2.get_pixel(x, y)
        else
          assert_equal Color.new(0, 0, 0, 0), texture2.get_pixel(x, y)
        end
      end
    end
  end
  
  def test_render_texture_scale
    texture = Texture.load("images/ruby")
    texture2 = Texture.new(texture.width, texture.height)
    texture2.render_texture(texture, 0, 0, :scale_x => 2, :scale_y => 2)
    texture2.height.times do |y|
      texture2.width.times do |x|
        p1 = texture.get_pixel(x / 2, y / 2)
        p2 = texture2.get_pixel(x, y)
        assert_equal p1, p2
      end
    end
    w = texture.width
    h = texture.height
    texture2.clear
    texture2.render_texture(texture, w, h, :scale_x => -1, :scale_y => -1)
    texture2.height.times do |y|
      texture2.width.times do |x|
        p1 = texture.get_pixel(w - x - 1, h - y - 1)
        p2 = texture2.get_pixel(x, y)
        assert_equal p1, p2
      end
    end
    texture2.clear
    texture2.render_texture(texture, w / 2, 0, :scale_x => -1)
    texture2.height.times do |y|
      texture2.width.times do |x|
        if x < w / 2
          p1 = texture.get_pixel(w / 2 - x - 1, y)
          p2 = texture2.get_pixel(x, y)
          assert_equal p1, p2
        else
          assert_equal Color.new(0, 0, 0, 0), texture2.get_pixel(x, y)
        end
      end
    end
  end
  
  def test_render_texture_src_rect
    texture = Texture.load("images/ruby")
    texture2 = Texture.new(texture.width, texture.height)
    texture2.render_texture(texture, 10, 11, {
      :src_x => 12, :src_y => 13, :src_width => 14, :src_height => 15
    })
    texture2.height.times do |y|
      texture2.width.times do |x|
        if 10 <= x and 11 <= y and x < 24 and y < 26
          assert_equal texture.get_pixel(x - 10 + 12, y - 11 + 13), texture2.get_pixel(x, y)
        else
          assert_equal Color.new(0, 0, 0, 0), texture2.get_pixel(x, y)
        end
      end
    end
  end
  
  def test_render_texture_alpha
    texture = Texture.load("images/ruby")
    texture2 = Texture.new(texture.width, texture.height)
    texture2.render_texture(texture, 0, 0, :alpha => 128)
    texture2.height.times do |y|
      texture2.width.times do |x|
        p1 = texture.get_pixel(x, y)
        p2 = texture2.get_pixel(x, y)
        assert_in_delta p1.red / 2,   p2.red,   1
        assert_in_delta p1.green / 2, p2.green, 1
        assert_in_delta p1.blue / 2,  p2.blue,  1
        assert_in_delta p1.alpha / 2, p2.alpha, 1
      end
    end
    texture2.clear
    texture2.render_texture(texture, 0, 0, :alpha => 64)
    texture2.height.times do |y|
      texture2.width.times do |x|
        p1 = texture.get_pixel(x, y)
        p2 = texture2.get_pixel(x, y)
        assert_in_delta p1.red / 4,   p2.red,   1
        assert_in_delta p1.green / 4, p2.green, 1
        assert_in_delta p1.blue / 4,  p2.blue,  1
        assert_in_delta p1.alpha / 4, p2.alpha, 1
      end
    end
    texture2.clear
    texture2.render_texture(texture, 0, 0, :alpha => 192)
    texture2.height.times do |y|
      texture2.width.times do |x|
        p1 = texture.get_pixel(x, y)
        p2 = texture2.get_pixel(x, y)
        assert_in_delta p1.red * 3 / 4,   p2.red,   1
        assert_in_delta p1.green * 3 / 4, p2.green, 1
        assert_in_delta p1.blue * 3 / 4,  p2.blue,  1
        assert_in_delta p1.alpha * 3 / 4, p2.alpha, 1
      end
    end
  end
  
  def test_render_texture_blend_type
    # alpha
    texture = Texture.load("images/ruby")
    texture2 = Texture.new(texture.width, texture.height)
    texture2.render_texture(texture, 0, 0, :blend_type => :alpha)
    texture2.height.times do |y|
      texture2.width.times do |x|
        p1 = texture.get_pixel(x, y)
        p2 = texture2.get_pixel(x, y)
        assert_equal p1, p2
      end
    end
    # add
    texture2.fill Color.new(100, 110, 120, 130)
    texture2.render_texture(texture, 0, 0, :blend_type => :add, :alpha => 128)
    texture2.height.times do |y|
      texture2.width.times do |x|
        p1 = texture.get_pixel(x, y)
        p2 = texture2.get_pixel(x, y)
        case p1.alpha
        when 255
          assert_in_delta 100 + p1.red / 2,   p2.red,   1
          assert_in_delta 110 + p1.green / 2, p2.green, 1
          assert_in_delta 120 + p1.blue / 2,  p2.blue,  1
        when 0
          assert_equal 100, p2.red
          assert_equal 110, p2.green
          assert_equal 120, p2.blue
        else
          flunk
        end
        assert_in_delta [130, p1.alpha / 2].max, p2.alpha, 1
      end
    end
    # sub
    texture2.fill Color.new(100, 110, 120, 130)
    texture2.render_texture(texture, 0, 0, :blend_type => :sub, :alpha => 128)
    texture2.height.times do |y|
      texture2.width.times do |x|
        p1 = texture.get_pixel(x, y)
        p2 = texture2.get_pixel(x, y)
        case p1.alpha
        when 255
          assert_in_delta [100 - p1.red / 2, 0].max,   p2.red,   1
          assert_in_delta [110 - p1.green / 2, 0].max, p2.green, 1
          assert_in_delta [120 - p1.blue / 2, 0].max,  p2.blue,  1
        when 0
          assert_equal 100, p2.red
          assert_equal 110, p2.green
          assert_equal 120, p2.blue
        else
          flunk
        end
        assert_in_delta [130, p1.alpha / 2].max, p2.alpha, 1
      end
    end
  end
  
  def test_render_texture_tone
    texture = Texture.load("images/ruby")
    texture2 = Texture.new(texture.width, texture.height)
    texture2.render_texture(texture, 0, 0, {
      :tone_red => 0, :tone_green => 0, :tone_blue => 0, :saturation => 255,
    })
    texture2.height.times do |y|
      texture2.width.times do |x|
        p1 = texture.get_pixel(x, y)
        p2 = texture2.get_pixel(x, y)
        assert_equal p1, p2
      end
    end
    texture2.clear
    texture2 = Texture.new(texture.width, texture.height)
    texture2.render_texture(texture, 0, 0, {
      :tone_red => 0, :tone_green => 0, :tone_blue => 0, :saturation => 128,
    })
    texture2.height.times do |y|
      texture2.width.times do |x|
        p1 = texture.get_pixel(x, y)
        p2 = texture2.get_pixel(x, y)
        case p1.alpha
        when 255
          gray = 0.3 * p1.red + 0.59 * p1.green + 0.11 * p1.blue
          assert_in_delta (gray + p1.red) / 2,   p2.red,   1
          assert_in_delta (gray + p1.green) / 2, p2.green, 1
          assert_in_delta (gray + p1.blue) / 2,  p2.blue,  1
        when 0
          assert_equal 0, p2.red
          assert_equal 0, p2.green
          assert_equal 0, p2.blue
        end
        assert_equal p1.alpha, p2.alpha
      end
    end
    texture2.clear
    texture2.render_texture(texture, 0, 0, {
      :tone_red => 0, :tone_green => 0, :tone_blue => 0, :saturation => 0,
    })
    texture2.height.times do |y|
      texture2.width.times do |x|
        p1 = texture.get_pixel(x, y)
        p2 = texture2.get_pixel(x, y)
        case p1.alpha
        when 255
          assert(p2.red == p2.green && p2.green == p2.blue)
          gray = 0.3 * p1.red + 0.59 * p1.green + 0.11 * p1.blue
          assert_in_delta gray, p2.red, 1
        when 0
          assert_equal 0, p2.red
          assert_equal 0, p2.green
          assert_equal 0, p2.blue
        end
        assert_equal p1.alpha, p2.alpha
      end
    end
    texture2.clear
    texture2.render_texture(texture, 0, 0, {
      :tone_red => 64, :tone_green => 128, :tone_blue => 192,
    })
    texture2.height.times do |y|
      texture2.width.times do |x|
        p1 = texture.get_pixel(x, y)
        p2 = texture2.get_pixel(x, y)
        case p1.alpha
        when 255
          assert_in_delta (255 + p1.red * 3) / 4,  p2.red,   1
          assert_in_delta (255 + p1.green) / 2,    p2.green, 1
          assert_in_delta (255 * 3 + p1.blue) / 4, p2.blue,  1
        when 0
          assert_equal 0, p2.red
          assert_equal 0, p2.green
          assert_equal 0, p2.blue
        end
        assert_equal p1.alpha, p2.alpha
      end
    end
    texture2.clear
    texture2.render_texture(texture, 0, 0, {
      :tone_red => -192, :tone_green => -128, :tone_blue => -64,
    })
    texture2.height.times do |y|
      texture2.width.times do |x|
        p1 = texture.get_pixel(x, y)
        p2 = texture2.get_pixel(x, y)
        case p1.alpha
        when 255
          assert_in_delta p1.red / 4,      p2.red,   1
          assert_in_delta p1.green / 2,    p2.green, 1
          assert_in_delta p1.blue * 3 / 4, p2.blue,  1
        when 0
          assert_equal 0, p2.red
          assert_equal 0, p2.green
          assert_equal 0, p2.blue
        end
        assert_equal p1.alpha, p2.alpha
      end
    end
  end
  
  def test_render_texture_self
    texture = Texture.load("images/ruby")
    texture2 = Texture.new(texture.width, texture.height)
    texture2.render_texture(texture, 0, 0)
    texture2.render_texture(texture2, 10, 10)
    texture2.height.times do |y|
      texture2.width.times do |x|
        if (x < 10 or y < 10) or texture.get_pixel(x - 10, y - 10).alpha == 0
          assert_equal texture.get_pixel(x, y), texture2.get_pixel(x, y)
        else
          assert_equal texture.get_pixel(x - 10, y - 10), texture2.get_pixel(x, y)
        end
      end
    end
  end

end
