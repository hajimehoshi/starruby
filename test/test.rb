#!/usr/bin/env ruby

if "1.9.0" <= RUBY_VERSION
  FrozenError = RuntimeError
else
  FrozenError = TypeError
  class Fixnum
    def ord
      self
    end
  end
end

require "starruby"
require "test/unit"

include StarRuby

class StarRubyTest < Test::Unit::TestCase

  def test_version
    assert_equal "0.1.14", StarRuby::VERSION
  end

end

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

  def test_color_type
    assert_raise TypeError do
      Color.new(nil, 2, 3, 4)
    end
    assert_raise TypeError do
      Color.new(1, nil, 3, 4)
    end
    assert_raise TypeError do
      Color.new(1, 2, nil, 4)
    end
    assert_raise TypeError do
      Color.new(1, 2, 3, false)
    end
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

  def test_exist
    case RUBY_PLATFORM
    when /mswin32|cygwin|mingw32|bccwin32|interix|djgpp/
      # Windows
      assert_equal true,  Font.exist?("Arial")
      assert_equal false, Font.exist?("arial.ttf")
      assert_equal false, Font.exist?("arial.ttc")
      assert_equal (File.exist?("./arial.ttf") or File.exist?("./arial.ttc")), Font.exist?("arial")
      assert_equal (File.exist?("./Arial.ttf") or File.exist?("./Arial.ttc")), Font.exist?("./Arial")
      assert_equal (File.exist?("./arial.ttf") or File.exist?("./arial.ttc")), Font.exist?("./arial")
      assert_equal false, Font.exist?("msgothic")
      assert_equal false, Font.exist?("msgothic.ttf")
      assert_equal false, Font.exist?("msgothic.ttc")
      assert_equal true,  Font.exist?("Arial")
      assert_equal true,  Font.exist?("ＭＳ ゴシック")
      assert_equal false, Font.exist?("notfont")
      assert_equal false, Font.exist?("notfont.ttf")
      assert_equal false, Font.exist?("notfont.ttc")
    when /linux/
      # Linux
      assert_equal true,  Font.exist?("FreeSans")
      assert_equal true,  Font.exist?("FreeSans")
      assert_equal false, Font.exist?("FreeSans:style=Bold")
      assert_equal true,  Font.exist?("FreeSans, Bold")
      assert_equal true,  Font.exist?("FreeSans, BoldOblique")
      assert_equal false, Font.exist?("FreeSans, NotStyle")
      assert_equal true,  Font.exist?("FreeSans ,Bold")
      assert_equal true,  Font.exist?("FreeSans ,")
      assert_equal false, Font.exist?("FreeSans.ttf")
      assert_equal false, Font.exist?("FreeSans.ttc")
    when /darwin/
      # Mac OS X
      assert_equal true,  Font.exist?("Helvetica")
      assert_equal true,  Font.exist?("Helvetica, Regular")
      assert_equal true,  Font.exist?("Helvetica, Oblique")
      assert_equal false, Font.exist?("Helvetica, NotStyle")
    end
    assert_raise ArgumentError do
      Font.exist?("fonts/maybefont")
    end
    assert_equal true, Font.exist?("fonts/ORANGEKI")
    assert_equal true, Font.exist?("fonts/maybefont2")
  end

  def test_exist_type
    assert_raise TypeError do
      Font.exist?(nil)
    end
  end

  def test_new
    if Font.exist?("Arial")
      font = Font.new("Arial", 16)
      assert_equal "Arial", font.name
    elsif Font.exist?("FreeSans")
      font = Font.new("FreeSans", 16)
      assert_equal "FreeSans", font.name
    elsif Font.exist?("Helvetica Neue")
      font = Font.new("Helvetica Neue", 16)
      assert_equal "Helvetica Neue", font.name
    else
      flunk
    end
    assert_equal 16, font.size
    assert_equal false, font.bold?
    assert_equal false, font.italic?
    if Font.exist?("MS UI Gothic")
      font = Font.new("MS UI Gothic", 12, {
                        :ttc_index => 1, :bold => true, :italic => true
                      }) # :ttc_index is ignored
      assert_equal 12, font.size
      assert_equal "MS UI Gothic", font.name
      assert_equal true, font.bold?
      assert_equal true, font.italic?
    end
    assert_raise Errno::ENOENT do
      Font.new("notfont", 12)
    end
    assert_raise ArgumentError do
      Font.new("fonts/maybefont", 12)
    end
    assert_raise StarRubyError do
      Font.new("fonts/maybefont2", 12)
    end
  end

  def test_new_type
    assert_raise TypeError do
      Font.new(nil, 12)
    end
    assert_raise TypeError do
      Font.new("fonts/ORANGEKI", nil)
    end
    assert_raise TypeError do
      Font.new("fonts/ORANGEKI", 12, false)
    end
  end

  def test_get_size
    if Font.exist?("ＭＳ ゴシック")
      font = Font.new("ＭＳ ゴシック", 12)
      assert_equal "MS Gothic", font.name
      assert_equal [6, 13], font.get_size("A")
      assert font.get_size("A").frozen?
      h = font.get_size("A")[1]
      assert_equal [78, h], font.get_size("Hello, World!")
      assert_equal [60, h], font.get_size("こんにちは")
      assert_equal [30, h], font.get_size("aaa&a")
    end
    if Font.exist?("Arial")
      font = Font.new("Arial", 16)
    elsif Font.exist?("FreeSans")
      font = Font.new("FreeSans", 16)
    elsif Font.exist?("Helvetica Neue")
      font = Font.new("Helvetica Neue", 16)
    else
      flunk
    end
    size = font.get_size("AAAAAAAAAAAAAA");
    size[0] # No Exception
    size[1] # No Exception
  end

  def test_get_size_type
    font = Font.new("fonts/ORANGEKI", 12)
    assert_raise TypeError do
      font.get_size(nil)
    end
  end

end

class GameTest < Test::Unit::TestCase

  def test_game
    assert_equal false, Game.running?
    assert_equal "foo", (Game.title = "foo")
    assert_equal "foo", Game.title
    assert_equal 30, Game.fps;
    assert_equal 31, (Game.fps = 31)
    assert_equal 31, Game.fps
    Game.run(320, 240) do
      assert_equal true, Game.running?
      assert_equal "bar", (Game.title = "bar")
      assert_equal "bar", Game.title
      assert_equal 32, (Game.fps = 32)
      assert_equal 32, Game.fps
      assert_raise StarRubyError do
        Game.run(320, 240) {}
      end
      Game.terminate
      assert_equal true, Game.running?
    end
    assert_equal false, Game.running?
    assert_equal "baz", (Game.title = "baz")
    assert_equal "baz", Game.title
    assert_equal 33, (Game.fps = 33)
    assert_equal 33, Game.fps
  end

  def test_game_type
    assert_raise TypeError do
      Game.title = nil
    end
    assert_raise TypeError do
      Game.fps = nil
    end
  end

  def test_run
    Game.run(320, 240, :window_scale => 2) do
      assert_equal [320, 240], Game.screen.size
      Game.terminate
    end
  end

  def test_run_type
    assert_raise TypeError do
      Game.run(nil, 240) {}
    end
    assert_raise TypeError do
      Game.run(320, nil) {}
    end
    assert_raise TypeError do
      Game.run(320, 240, false) {}
    end
    assert_raise TypeError do
      Game.run(320, 240, :window_scale => false) {}
    end
  end
  
  def test_running
    assert_equal false, Game.running?
    Game.run(320, 240) do
      assert_equal true, Game.running?
      Game.terminate
    end
    assert_equal false, Game.running?
    Game.run(320, 240) do
      assert_equal true, Game.running?
      Game.terminate
    end
    assert_equal false, Game.running?
  end
  
  def test_screen
    assert_nil Game.screen
    Game.run(320, 240) do
      begin
        assert_kind_of Texture, Game.screen
        assert_equal [320, 240], Game.screen.size
      ensure
        Game.terminate
      end
    end
    Game.run(123, 456) do
      begin
        assert_kind_of Texture, Game.screen
        assert_equal [123, 456], Game.screen.size
      ensure
        Game.terminate
      end
    end
    assert_nil Game.screen
    begin
      Game.run(320, 240) do
        assert_not_nil Game.screen
        raise RuntimeError, "runtime error"
      end
    rescue RuntimeError
    end
    assert_nil Game.screen
  end
  
end

class TextureTest < Test::Unit::TestCase
  
  def test_new
    texture = Texture.new(123, 456)
    assert_equal 123, texture.width
    assert_equal 456, texture.height
    assert_equal [123, 456], texture.size
    assert_equal true, texture.size.frozen?
    
    assert_raise ArgumentError do
      Texture.new(123, -456)
    end
    assert_raise ArgumentError do
      Texture.new(-123, 456)
    end
    assert_raise ArgumentError do
      Texture.new(-123, -456)
    end
  end
  
  def test_new_disposed
    texture = Texture.new(123, 456)
    texture.dispose
    assert_raise RuntimeError do
      texture.width
    end
    assert_raise RuntimeError do
      texture.height
    end
    assert_raise RuntimeError do
      texture.size
    end
  end
  
  def test_new_type
    assert_raise TypeError do
      Texture.new(nil, 456)
    end
    assert_raise TypeError do
      Texture.new(123, nil)
    end
  end
  
  def test_load
    texture = Texture.load("images/ruby.png")
    assert_equal 49, texture.width
    assert_equal 49, texture.height
    assert_equal [49, 49], texture.size
    
    assert_raise Errno::ENOENT do
      Texture.load("invalid_path/foo.png")
    end
    
    assert_raise Errno::ENOENT do
      Texture.load("images/not_existed.png")
    end
    
    assert_not_nil Texture.load("images/ruby");
    assert_not_nil Texture.load("images/rubypng");
    assert_raise Errno::ENOENT do
      assert_not_nil Texture.load("images/foo");
    end
    assert_not_nil Texture.load("images/foo.bar");
    assert_not_nil Texture.load("images/foo.bar.png");
    
    assert_raise StarRubyError do
      Texture.load("images/not_image");
    end
    
    assert_raise StarRubyError do
      Texture.load("images/not_image.txt");
    end
    
    assert_raise ArgumentError do
      Texture.load("images/ambiguous");
    end
    
    Texture.load("images/_ruby")
    Texture.load("images/ruby_")
  end
  
  def test_load_type
    assert_raise TypeError do
      Texture.load(nil)
    end
  end
  
  def test_load_various_png
    texture8 = Texture.load("images/ruby8")
    assert_equal 0, texture8.get_pixel(1, 1).alpha
    assert_equal Color.new(252, 239, 239, 255), texture8.get_pixel(58, 2)
    texture8 = Texture.load("images/ruby8_without_alpha")
    assert_equal 255, texture8.get_pixel(1, 1).alpha
    assert_equal Color.new(252, 239, 239, 255), texture8.get_pixel(58, 2)
    texture8 = Texture.load("images/ruby8_16colors")
    assert_equal 0, texture8.get_pixel(1, 1).alpha
    assert_equal Color.new(251, 246, 243, 255), texture8.get_pixel(58, 2)
    texture16 = Texture.load("images/ruby16")
    assert_equal Color.new(245, 245, 245, 186), texture16.get_pixel(58, 2)
    texture32 = Texture.load("images/ruby32")
    assert_equal Color.new(252, 242, 242, 186), texture32.get_pixel(58, 2)
    assert_raise StarRubyError do
      texture32 = Texture.load("images/ruby32_interlace")
    end
  end

  def test_transform_in_perspective
    assert_raise TypeError do
      Texture.transform_in_perspective(false, 0, 0)
    end
    assert_raise TypeError do
      Texture.transform_in_perspective(0, false, 0)
    end
    assert_raise TypeError do
      Texture.transform_in_perspective(0, 0, false)
    end
    assert_raise TypeError do
      Texture.transform_in_perspective(0, 0, 0, false)
    end
    options = {
      :camera_x      => 0,
      :camera_y      => 0,
      :camera_height => 100,
      :camera_yaw    => 0,
      :camera_pitch  => 0,
      :camera_roll   => 0,
      :distance      => 100,
      :intersection_x   => 0,
      :intersection_y   => 0,
    }
    assert_equal [0, 50, 0.5], Texture.transform_in_perspective(0, -200, 0, options)
    options = {
      :camera_x      => 0,
      :camera_y      => 0,
      :camera_height => 100,
      :camera_yaw    => 0,
      :camera_pitch  => 0,
      :camera_roll   => 0,
      :distance      => 100,
      :intersection_x   => 12,
      :intersection_y   => 34,
    }
    assert_equal [12, 84, 0.5], Texture.transform_in_perspective(0, -200, 0, options)
    options = {
      :camera_x      => 0,
      :camera_y      => 0,
      :camera_height => 100,
      :camera_yaw    => 0,
      :camera_pitch  => 0,
      :camera_roll   => 0,
      :distance      => 100,
      :intersection_x   => 0,
      :intersection_y   => 0,
    }
    assert_equal [100, 50, 0.5], Texture.transform_in_perspective(200, -200, 0, options)
    options = {
      :camera_x      => 0,
      :camera_y      => 0,
      :camera_height => 100,
      :camera_yaw    => 0,
      :camera_pitch  => 0,
      :camera_roll   => 0,
      :distance      => 100,
      :intersection_x   => 0,
      :intersection_y   => 0,
    }
    assert_equal [50, 25, 0.25], Texture.transform_in_perspective(200, -400, 0, options)
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
    texture.dispose
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
    assert_raise RuntimeError do
      texture.get_pixel(0, 1)
    end
  end
  
  def test_set_pixel_frozen
    texture = Texture.new(3, 3)
    texture.freeze
    assert_raise FrozenError do
      texture.set_pixel(0, 1, Color.new(31, 41, 59, 26))
    end
  end
  
  def test_set_pixel_disposed
    texture = Texture.new(3, 3)
    texture.dispose
    assert_raise RuntimeError do
      texture.set_pixel(0, 1, Color.new(31, 41, 59, 26))
    end
  end
  
  def test_get_and_set_pixel_type
    texture = Texture.new(3, 3)
    assert_raise TypeError do
      texture.get_pixel(nil, 0)
    end
    assert_raise TypeError do
      texture.get_pixel(0, nil)
    end
    assert_raise TypeError do
      texture.set_pixel(nil, 0, Color.new(0, 0, 0))
    end
    assert_raise TypeError do
      texture.set_pixel(0, nil, Color.new(0, 0, 0))
    end
    assert_raise TypeError do
      texture.set_pixel(0, 0, nil)
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
    assert_raise FrozenError do
      texture.clear
    end
  end
  
  def test_clear_disposed
    texture = Texture.load("images/ruby")
    texture.dispose
    assert_raise RuntimeError do
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
    assert_raise FrozenError do
      texture.fill(Color.new(31, 41, 59, 26))
    end
  end
  
  def test_fill_disposed
    texture = Texture.load("images/ruby")
    texture.dispose
    assert_raise RuntimeError do
      texture.fill(Color.new(31, 41, 59, 26))
    end
  end
  
  def test_fill_type
    texture = Texture.load("images/ruby")
    assert_raise TypeError do
      texture.fill(nil)
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
    texture = Texture.new(900, 100)
    assert_raise ArgumentError do
      texture.fill_rect(-16, 16, 16, 16, Color.new(0, 0, 0, 0))
    end
    assert_raise ArgumentError do
      texture.fill_rect(16, -16, 16, 16, Color.new(0, 0, 0, 0))
    end
    assert_raise ArgumentError do
      texture.fill_rect(16, 16, texture.width + 16, 16, Color.new(0, 0, 0, 0))
    end
    assert_raise ArgumentError do
      texture.fill_rect(16, 16, 16, texture.height + 16, Color.new(0, 0, 0, 0))
    end
    texture = orig_texture.clone
    texture.fill_rect(1, 0, texture.width - 1, texture.height, Color.new(12, 34, 56, 78))
    texture.height.times do |y|
      texture.width.times do |x|
        if 1 <= x
          assert_equal Color.new(12, 34, 56, 78), texture.get_pixel(x, y)
        else
          assert_equal orig_texture.get_pixel(x, y), texture.get_pixel(x, y)
        end
      end
    end
    texture = orig_texture.clone
    texture.fill_rect(0, 1, texture.width, texture.height - 1, Color.new(12, 34, 56, 78))
    texture.height.times do |y|
      texture.width.times do |x|
        if 1 <= y
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
    assert_raise FrozenError do
      texture.fill_rect(10, 11, 12, 13, Color.new(12, 34, 56, 78))
    end
  end
  
  def test_fill_rect_disposed
    texture = Texture.load("images/ruby")
    texture.dispose
    assert_raise RuntimeError do
      texture.fill_rect(10, 11, 12, 13, Color.new(12, 34, 56, 78))
    end
  end
  
  def test_fill_rect_type
    texture = Texture.load("images/ruby")
    assert_raise TypeError do
      texture.fill_rect(nil, 11, 12, 13, Color.new(12, 34, 56, 78))
    end
    assert_raise TypeError do
      texture.fill_rect(10, nil, 12, 13, Color.new(12, 34, 56, 78))
    end
    assert_raise TypeError do
      texture.fill_rect(10, 11, nil, 13, Color.new(12, 34, 56, 78))
    end
    assert_raise TypeError do
      texture.fill_rect(10, 11, 12, nil, Color.new(12, 34, 56, 78))
    end
    assert_raise TypeError do
      texture.fill_rect(10, 11, 12, 13, nil)
    end
  end
  
  def test_change_hue
    texture = Texture.load("images/ruby")
    orig_texture = texture.clone
    texture = orig_texture.change_hue(0)
    texture.height.times do |y|
      texture.width.times do |x|
        assert_equal orig_texture.get_pixel(x, y), texture.get_pixel(x, y)
      end
    end
    texture = orig_texture.clone
    texture.change_hue!(0)
    texture.height.times do |y|
      texture.width.times do |x|
        assert_equal orig_texture.get_pixel(x, y), texture.get_pixel(x, y)
      end
    end
    texture = orig_texture.change_hue(Math::PI * 2 / 3)
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
    texture.change_hue!(Math::PI * 2 / 3)
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
    texture = orig_texture.change_hue(Math::PI * 4 / 3)
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
    texture = orig_texture.clone
    texture.change_hue!(Math::PI * 4 / 3)
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
    texture.change_hue(Math::PI)
    assert_raise FrozenError do
      texture.change_hue!(Math::PI)
    end
  end

  def test_change_hue_disposed
    texture = Texture.load("images/ruby")
    texture.dispose
    assert_raise RuntimeError do
      texture.change_hue(Math::PI)
    end
    assert_raise RuntimeError do
      texture.change_hue!(Math::PI)
    end
  end

  def test_change_hue_type
    texture = Texture.load("images/ruby")
    assert_raise TypeError do
      texture.change_hue(nil)
    end
    assert_raise TypeError do
      texture.change_hue!(nil)
    end
  end

  def test_render_in_perspective
    texture = Texture.load("images/ruby")
    texture2 = Texture.new(100, 100)
    texture2.render_in_perspective(texture)
    assert_raise TypeError do
      texture2.render_in_perspective(nil)
    end
    [:camera_x, :camera_y, :camera_height, :camera_yaw, :camera_pitch, :camera_roll,
     :distance, :intersection_x, :intersection_y].each do |key|
      assert_raise TypeError do
        texture2.render_in_perspective(texture, key => false)
      end
    end
    assert_raise RuntimeError do
      texture2.render_in_perspective(texture2) # self
    end
  end

  def test_render_in_perspective_disposed
    texture = Texture.load("images/ruby")
    texture2 = Texture.new(100, 100)
    texture.dispose
    assert_raise RuntimeError do
      texture2.render_in_perspective(texture)
    end
    texture = Texture.load("images/ruby")
    texture2 = Texture.new(100, 100)
    texture2.dispose
    assert_raise RuntimeError do
      texture2.render_in_perspective(texture)
    end
  end
  
  def test_render_in_perspective_frozen
    texture = Texture.load("images/ruby")
    texture2 = Texture.new(100, 100)
    texture.freeze
    texture2.render_in_perspective(texture)
    texture2.freeze
    assert_raise FrozenError do
      texture2.render_in_perspective(texture)
    end
  end

  def test_render_line
    texture = Texture.load("images/ruby")
    texture2 = texture.dup
    texture2.render_line(12, 34, 12, 56, Color.new(12, 34, 56, 255))
    texture.height.times do |j|
      texture.width.times do |i|
        p1 = texture.get_pixel(i, j)
        p2 = texture2.get_pixel(i, j)
        if i == 12 and 34 <= j and j <= 56
          assert_equal Color.new(12, 34, 56, 255), p2
        else
          assert_equal p1, p2
        end
      end
    end
    texture2 = texture.dup
    texture2.render_line(12, 56, 12, 34, Color.new(12, 34, 56, 255))
    texture.height.times do |j|
      texture.width.times do |i|
        p1 = texture.get_pixel(i, j)
        p2 = texture2.get_pixel(i, j)
        if i == 12 and 34 <= j and j <= 56
          assert_equal Color.new(12, 34, 56, 255), p2
        else
          assert_equal p1, p2
        end
      end
    end
    texture2 = texture.dup
    texture2.render_line(12, 34, 56, 34, Color.new(12, 34, 56, 255))
    texture.height.times do |j|
      texture.width.times do |i|
        p1 = texture.get_pixel(i, j)
        p2 = texture2.get_pixel(i, j)
        if j == 34 and 12 <= i and i <= 56
          assert_equal Color.new(12, 34, 56, 255), p2
        else
          assert_equal p1, p2
        end
      end
    end
    texture2 = texture.dup
    texture2.render_line(56, 34, 12, 34, Color.new(12, 34, 56, 255))
    texture.height.times do |j|
      texture.width.times do |i|
        p1 = texture.get_pixel(i, j)
        p2 = texture2.get_pixel(i, j)
        if j == 34 and 12 <= i and i <= 56
          assert_equal Color.new(12, 34, 56, 255), p2
        else
          assert_equal p1, p2
        end
      end
    end
    texture2 = texture.dup
    texture2.render_line(12, 34, 56, 34, Color.new(12, 34, 56, 78))
    texture.height.times do |j|
      texture.width.times do |i|
        p1 = texture.get_pixel(i, j)
        p2 = texture2.get_pixel(i, j)
        if j == 34 and 12 <= i and i <= 56
          if 0 < p1.alpha
            assert_in_delta (12 * 78 + p1.red   * (255 - 78)).quo(255), p2.red,   2
            assert_in_delta (34 * 78 + p1.green * (255 - 78)).quo(255), p2.green, 2
            assert_in_delta (56 * 78 + p1.blue  * (255 - 78)).quo(255), p2.blue,  2
            assert_equal [p1.alpha, 78].max, p2.alpha
          else
            assert_equal Color.new(12, 34, 56, 78), p2
          end
        else
          assert_equal p1, p2
        end
      end
    end
  end

  def test_render_line_disposed
    texture = Texture.load("images/ruby")
    texture2 = texture.dup
    texture2.dispose
    assert_raise RuntimeError do
      texture2.render_line(12, 34, 12, 56, Color.new(12, 34, 56, 255))
    end
  end

  def test_render_line_frozen
    texture = Texture.load("images/ruby")
    texture2 = texture.dup
    texture2.freeze
    assert_raise FrozenError do
      texture2.render_line(12, 34, 12, 56, Color.new(12, 34, 56, 255))
    end
  end

  def test_render_pixel
    texture = Texture.load("images/ruby")
    texture2 = texture.dup
    texture2.render_pixel(12, 34, Color.new(12, 34, 56, 78))
    texture.height.times do |j|
      texture.width.times do |i|
        p1 = texture.get_pixel(i, j)
        p2 = texture2.get_pixel(i, j)
        if i == 12 and j == 34
          if 0 < p1.alpha
            assert_in_delta (12 * 78 + p1.red   * (255 - 78)).quo(255), p2.red,   2
            assert_in_delta (34 * 78 + p1.green * (255 - 78)).quo(255), p2.green, 2
            assert_in_delta (56 * 78 + p1.blue  * (255 - 78)).quo(255), p2.blue,  2
            assert_equal [p1.alpha, 78].max, p2.alpha
          else
            assert_equal Color.new(12, 34, 56, 78), p2
          end
        else
          assert_equal p1, p2
        end
      end
    end
    texture2 = texture.dup
    assert_raise ArgumentError do
      texture2.render_pixel(-1, 0, Color.new(255, 255, 255))
    end
    texture2 = texture.dup
    assert_raise ArgumentError do
      texture2.render_pixel(0, -1, Color.new(255, 255, 255))
    end
    texture2 = texture.dup
    assert_raise ArgumentError do
      texture2.render_pixel(texture2.width, 0, Color.new(255, 255, 255))
    end
    texture2 = texture.dup
    assert_raise ArgumentError do
      texture2.render_pixel(0, texture2.height, Color.new(255, 255, 255))
    end
  end

  def test_render_pixel_disposed
    texture = Texture.load("images/ruby")
    texture2 = texture.dup
    texture2.dispose
    assert_raise RuntimeError do
      texture2.render_pixel(12, 34, Color.new(12, 34, 56, 78))
    end
  end

  def test_render_pixel_frozen
    texture = Texture.load("images/ruby")
    texture2 = texture.dup
    texture2.freeze
    assert_raise FrozenError do
      texture2.render_pixel(12, 34, Color.new(12, 34, 56, 78))
    end
  end

  def test_render_rect
    texture = Texture.load("images/ruby")
    texture2 = texture.dup
    texture2.render_rect(12, 34, 5, 6, Color.new(12, 34, 56, 78))
    texture.height.times do |j|
      texture.width.times do |i|
        p1 = texture.get_pixel(i, j)
        p2 = texture2.get_pixel(i, j)
        if 12 <= i and i < 12 + 5 and 34 <= j and j < 34 + 6
          if 0 < p1.alpha
            assert_in_delta (12 * 78 + p1.red   * (255 - 78)).quo(255), p2.red,   2
            assert_in_delta (34 * 78 + p1.green * (255 - 78)).quo(255), p2.green, 2
            assert_in_delta (56 * 78 + p1.blue  * (255 - 78)).quo(255), p2.blue,  2
            assert_equal [p1.alpha, 78].max, p2.alpha
          else
            assert_equal Color.new(12, 34, 56, 78), p2
          end
        else
          assert_equal p1, p2
        end
      end
    end
    texture2 = texture.dup
    assert_raise ArgumentError do
      texture2.render_rect(-1, 0, 5, 5, Color.new(255, 255, 255))
    end
    texture2 = texture.dup
    assert_raise ArgumentError do
      texture2.render_rect(0, -1, 5, 5, Color.new(255, 255, 255))
    end
    texture2 = texture.dup
    assert_raise ArgumentError do
      texture2.render_rect(0, 0, texture2.width + 1, 0, Color.new(255, 255, 255))
    end
    texture2 = texture.dup
    assert_raise ArgumentError do
      texture2.render_rect(0, 0, 0, texture2.height + 1, Color.new(255, 255, 255))
    end
  end

  def test_render_rect_disposed
    texture = Texture.load("images/ruby")
    texture2 = texture.dup
    texture2.dispose
    assert_raise RuntimeError do
      texture2.render_rect(12, 34, 5, 6, Color.new(12, 34, 56, 78))
    end
  end

  def test_render_rect_frozen
    texture = Texture.load("images/ruby")
    texture2 = texture.dup
    texture2.freeze
    assert_raise FrozenError do
      texture2.render_rect(12, 34, 5, 6, Color.new(12, 34, 56, 78))
    end
  end

  def test_render_text
    texture = Texture.load("images/ruby")
    if Font.exist?("Arial")
      font = Font.new("Arial", 16)
    elsif Font.exist?("FreeSans")
      font = Font.new("FreeSans", 16)
    elsif Font.exist?("Helvetica Neue")
      font = Font.new("Helvetica Neue", 16)
    else
      flunk
    end
    color = Color.new(255, 255, 255)
    texture.render_text("A", 0, 0, font, color)
    texture.render_text("B", 10, 10, font, color, true)
    texture.render_text("AAAAAAAAAAAAAAAAAAAA", 10, 10, font, color, false)
    texture.render_text("", 0, 0, font, color)
  end
  
  def test_render_text_hello_world
    if Font.exist?("Arial") and not RUBY_PLATFORM =~ /linux/
      font = Font.new("Arial", 24)
      texture = Texture.load("images/hello_world")
      texture2 = Texture.new(320, 240)
      texture2.fill(Color.new(153, 204, 255))
      texture2.render_text("Hello, World", 0, 0, font, Color.new(153, 102, 51))
      texture2.render_text("Hello, World", 0, 24, font, Color.new(153, 102, 51), true)
      texture2.render_text("Hello, World", 0, 48, font, Color.new(153, 102, 51, 128))
      texture2.render_text("Hello, World", 0, 72, font, Color.new(153, 102, 51, 128), true)
      texture2.height.times do |j|
        texture2.width.times do |i|
          p1 = texture.get_pixel(i, j)
          p2 = texture2.get_pixel(i, j)
          assert_equal p1, p2
        end
      end
    end
  end
  
  def test_render_text_disposed
    texture = Texture.load("images/ruby")
    if Font.exist?("Arial")
      font = Font.new("Arial", 16)
    elsif Font.exist?("FreeSans")
      font = Font.new("FreeSans", 16)
    elsif Font.exist?("Helvetica Neue")
      font = Font.new("Helvetica Neue", 16)
    else
      flunk
    end
    color = Color.new(255, 255, 255)
    texture.dispose
    assert_raise RuntimeError do
      texture.render_text("A", 0, 0, font, color)
    end
  end
  
  def test_render_text_frozen
    texture = Texture.load("images/ruby")
    if Font.exist?("Arial")
      font = Font.new("Arial", 16)
    elsif Font.exist?("FreeSans")
      font = Font.new("FreeSans", 16)
    elsif Font.exist?("Helvetica Neue")
      font = Font.new("Helvetica Neue", 16)
    else
      flunk
    end
    color = Color.new(255, 255, 255)
    texture.freeze
    assert_raise FrozenError do
      texture.render_text("A", 0, 0, font, color)
    end
  end
  
  def test_render_text_type
    texture = Texture.load("images/ruby")
    if Font.exist?("Arial")
      font = Font.new("Arial", 16)
    elsif Font.exist?("FreeSans")
      font = Font.new("FreeSans", 16)
    elsif Font.exist?("Helvetica Neue")
      font = Font.new("Helvetica Neue", 16)
    else
      flunk
    end
    color = Color.new(255, 255, 255)
    assert_raise TypeError do
      texture.render_text(nil, 0, 0, font, color)
    end
    assert_raise TypeError do
      texture.render_text("aa", nil, 0, font, color)
    end
    assert_raise TypeError do
      texture.render_text("aa", 0, nil, font, color)
    end
    assert_raise TypeError do
      texture.render_text("aa", 0, 0, nil, color)
    end
    assert_raise TypeError do
      texture.render_text("aa", 0, 0, font, nil)
    end
  end
  
  def test_render_texture
    texture = Texture.load("images/ruby")
    texture2 = Texture.new(texture.width, texture.height)
    texture2.render_texture(texture, 0, 0)
    texture.height.times do |y|
      texture.width.times do |x|
        c1 = texture.get_pixel(x, y)
        c2 = texture2.get_pixel(x, y)
        assert_equal c1, c2
      end
    end
    texture2.fill(Color.new(128, 128, 128, 128))
    texture2.render_texture(texture, 0, 0)
    texture.height.times do |y|
      texture.width.times do |x|
        p1 = texture.get_pixel(x, y)
        p2 = texture2.get_pixel(x, y)
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
    [:alpha, :angle, :blend_type, :center_x, :center_y,
    :saturation, :scale_x, :scale_y, :src_height, :src_width,
    :src_x, :src_y, :tone_red, :tone_green, :tone_blue].each do |key|
      assert_raise TypeError, "#{key}" do
        texture2.render_texture(texture, 0, 0, key => false)
      end
    end
  end
  
  def test_render_texture_blending
    src_texture = Texture.new(256, 1)
    src_texture.width.times do |i|
      src_texture.set_pixel(i, 0, Color.new(127, 128, 129, i))
    end
    dst_texture = Texture.new(256, 1)
    dst_texture.render_texture(src_texture, 0, 0)
    src_texture.width.times do |i|
      assert_equal Color.new(127, 128, 129, i), dst_texture.get_pixel(i, 0)
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
          p1 = texture.get_pixel(x - 10, y - 11)
          p2 = texture2.get_pixel(x, y)
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
    texture2.height.times do |y|
      texture2.width.times do |x|
        if x < texture2.width - 12 and y < texture2.height - 13
          p1 = texture.get_pixel(x + 12, y + 13)
          p2 = texture2.get_pixel(x, y)
          if p2.alpha != 0
            assert_equal p1, p2
          else
            assert_equal Color.new(p1.red, p1.green, p1.blue, 0), p2
          end
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
        if p2.alpha != 0
          assert_equal p1, p2
        else
          assert_equal Color.new(p1.red, p1.green, p1.blue, 0), p2
        end
      end
    end
    texture2.clear
    texture2.render_texture(texture, 0, 0, :scale_x => 3, :scale_y => 4)
    texture2.height.times do |y|
      texture2.width.times do |x|
        p1 = texture.get_pixel(x / 3, y / 4)
        p2 = texture2.get_pixel(x, y)
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
    texture2.height.times do |y|
      texture2.width.times do |x|
        p1 = texture.get_pixel(w - x - 1, h - y - 1)
        p2 = texture2.get_pixel(x, y)
        if p2.alpha != 0
          assert_equal p1, p2
        else
          assert_equal Color.new(p1.red, p1.green, p1.blue, 0), p2
        end
      end
    end
    texture2.clear
    texture2.render_texture(texture, w / 2, 0, :scale_x => -1)
    texture2.height.times do |y|
      texture2.width.times do |x|
        if x < w / 2
          p1 = texture.get_pixel(w / 2 - x - 1, y)
          p2 = texture2.get_pixel(x, y)
          if p2.alpha != 0
            assert_equal p1, p2
          else
            assert_equal Color.new(p1.red, p1.green, p1.blue, 0), p2
          end
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
          p1 = texture.get_pixel(x - 10 + 12, y - 11 + 13)
          p2 = texture2.get_pixel(x, y)
          if p2.alpha != 0
            assert_equal p1, p2
          else
            assert_equal Color.new(0, 0, 0, 0), p2
          end
        else
          assert_equal Color.new(0, 0, 0, 0), texture2.get_pixel(x, y)
        end
      end
    end
  end

  def test_render_texture_src_rect_arg_error
    texture = Texture.load("images/ruby")
    texture2 = Texture.new(texture.width, texture.height)
    assert_raise ArgumentError do
      texture2.render_texture(texture, 0, 0, {
        :src_x => -100, :src_y => -100,
        :src_width => texture.width + 200,
        :src_height => texture.height + 200
      })
    end
    assert_raise ArgumentError do
      texture2.render_texture(texture, 0, 0, :src_x => -10)
    end
    assert_raise ArgumentError do
      texture2.render_texture(texture, 0, 0, :src_x => texture.width + 10)
    end
    assert_raise ArgumentError do
      texture2.render_texture(texture, 0, 0, :src_y => -10)
    end
    assert_raise ArgumentError do
      texture2.render_texture(texture, 0, 0, :src_y => texture.height + 10)
    end
    assert_raise ArgumentError do
      texture2.render_texture(texture, 0, 0, :src_x => 10, :src_width => -10)
    end
    assert_raise ArgumentError do
      texture2.render_texture(texture, 0, 0, {
        :src_x => 10, :src_width => texture.width - 5
      })
    end
    assert_raise ArgumentError do
      texture2.render_texture(texture, 0, 0, :src_y => 10, :src_height => -10)
    end
    assert_raise ArgumentError do
      texture2.render_texture(texture, 0, 0, {
        :src_y => 10, :src_height => texture.height - 5
      })
    end
    assert_raise ArgumentError do
      texture2.render_texture(texture, 0, 0, {
        :src_x => -10, :src_y => 0,
        :src_width => 10, :src_height => 10
      })
    end
    assert_raise ArgumentError do
      texture2.render_texture(texture, 0, 0, {
        :src_x => texture.width + 100, :src_y => texture.height + 100,
        :src_width => 100, :src_height => 100
      })
    end
  end
  
  def test_render_texture_alpha
    texture = Texture.load("images/ruby")
    texture2 = Texture.new(texture.width, texture.height)
    [64, 128, 192].each do |alpha|
      texture2.fill(Color.new(0, 0, 0, 1))
      texture2.render_texture(texture, 0, 0, :alpha => alpha)
      texture2.height.times do |y|
        texture2.width.times do |x|
          p1 = texture.get_pixel(x, y)
          p2 = texture2.get_pixel(x, y)
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
      p1 = texture.get_pixel(i, 0)
      p2 = texture2.get_pixel(i, 0)
      assert_in_delta p1.red   * alpha.quo(255) * i.quo(255), p2.red,   2
      assert_in_delta p1.green * alpha.quo(255) * i.quo(255), p2.green, 2
      assert_in_delta p1.blue  * alpha.quo(255) * i.quo(255), p2.blue,  2
      assert_in_delta [i * alpha.quo(255), 128].max, p2.alpha, 2
    end
    texture2.fill(Color.new(12, 34, 56, 0))
    alpha = 100
    texture2.render_texture(texture, 0, 0, :alpha => alpha)
    256.times do |i|
      p1 = texture.get_pixel(i, 0)
      p2 = texture2.get_pixel(i, 0)
      assert_equal p1.red,   p2.red
      assert_equal p1.green, p2.green
      assert_equal p1.blue,  p2.blue
      assert_in_delta i * alpha.quo(255), p2.alpha, 2
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
        if p2.alpha != 0
          assert_equal p1, p2
        else
          assert_equal Color.new(p1.red, p1.green, p1.blue, 0), p2
        end
      end
    end
    # add
    texture2.fill Color.new(100, 110, 120, 130)
    texture2.render_texture(texture, 0, 0, :blend_type => :add, :alpha => 128)
    texture2.height.times do |y|
      texture2.width.times do |x|
        p1 = texture.get_pixel(x, y)
        p2 = texture2.get_pixel(x, y)
        a = p1.alpha * 128.quo(255)
        assert_in_delta [p1.red   * a.quo(255) + 100, 255].min, p2.red,   2
        assert_in_delta [p1.green * a.quo(255) + 110, 255].min, p2.green, 2
        assert_in_delta [p1.blue  * a.quo(255) + 120, 255].min, p2.blue,  2
        assert_in_delta [a, 130].max, p2.alpha, 2
      end
    end
    # sub
    texture2.fill Color.new(100, 110, 120, 130)
    texture2.render_texture(texture, 0, 0, :blend_type => :sub, :alpha => 128)
    texture2.height.times do |y|
      texture2.width.times do |x|
        p1 = texture.get_pixel(x, y)
        p2 = texture2.get_pixel(x, y)
        a = p1.alpha * 128.quo(255)
        assert_in_delta [-p1.red   * a.quo(255) + 100, 0].max, p2.red,   2
        assert_in_delta [-p1.green * a.quo(255) + 110, 0].max, p2.green, 2
        assert_in_delta [-p1.blue  * a.quo(255) + 120, 0].max, p2.blue,  2
        assert_in_delta [a, 130].max, p2.alpha, 2
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
    texture2.height.times do |y|
      texture2.width.times do |x|
        p1 = texture.get_pixel(x, y)
        p2 = texture2.get_pixel(x, y)
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
    texture2.height.times do |y|
      texture2.width.times do |x|
        p1 = texture.get_pixel(x, y)
        p2 = texture2.get_pixel(x, y)
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
    texture2.height.times do |y|
      texture2.width.times do |x|
        p1 = texture.get_pixel(x, y)
        p2 = texture2.get_pixel(x, y)
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
    texture2.height.times do |y|
      texture2.width.times do |x|
        p1 = texture.get_pixel(x, y)
        p2 = texture2.get_pixel(x, y)
        assert_in_delta p1.red / 4,      p2.red,   1
        assert_in_delta p1.green / 2,    p2.green, 1
        assert_in_delta p1.blue * 3 / 4, p2.blue,  1
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
        if x < 10 or y < 10
          p1 = texture.get_pixel(x, y)
          p2 = texture2.get_pixel(x, y)
          assert_equal p1, p2
        else
          src = texture.get_pixel(x - 10, y - 10)
          dst = texture.get_pixel(x, y)
          p2 = texture2.get_pixel(x, y)
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

  def test_save
    texture = Texture.load("images/ruby")
    texture.save("images/saved_image.png", true)
    assert FileTest.file?("images/saved_image.png")
    texture2 = Texture.load("images/saved_image.png")
    assert_equal texture.size, texture2.size
    texture.height.times do |j|
      texture.width.times do |i|
        c1 = texture.get_pixel(i, j)
        c2 = texture2.get_pixel(i, j)
        assert_equal c1.red,   c2.red
        assert_equal c1.green, c2.green
        assert_equal c1.blue,  c2.blue
        assert_equal c1.alpha, c2.alpha
      end
    end
    texture.save("images/saved_image2.png")
    assert FileTest.file?("images/saved_image2.png")
    texture2 = Texture.load("images/saved_image2.png")
    assert_equal texture.size, texture2.size
    texture.height.times do |j|
      texture.width.times do |i|
        c1 = texture.get_pixel(i, j)
        c2 = texture2.get_pixel(i, j)
        assert_equal c1.red,   c2.red
        assert_equal c1.green, c2.green
        assert_equal c1.blue,  c2.blue
        assert_equal c1.alpha, c2.alpha
      end
    end
    if FileTest.exist?("images/saved_image.png")
      File.delete("images/saved_image.png")
    end
    if FileTest.exist?("images/saved_image2.png")
      File.delete("images/saved_image2.png")
    end
  end
  
  def test_save_type
    texture = Texture.load("images/ruby")
    assert_raise TypeError do
      texture.save(nil, true)
    end
  end
  
  def test_save_failed
    assert ! FileTest.exist?("images/foo")
    texture = Texture.load("images/ruby")
    assert_raise Errno::ENOENT do
      texture.save("images/foo/saved_image.png", true)
    end
  end
  
  def test_save_without_alpha
    texture = Texture.load("images/ruby")
    texture.save("images/saved_image.png", false)
    assert FileTest.file?("images/saved_image.png")
    texture2 = Texture.load("images/saved_image.png")
    assert_equal texture.size, texture2.size
    texture.height.times do |j|
      texture.width.times do |i|
        c1 = texture.get_pixel(i, j)
        c2 = texture2.get_pixel(i, j)
        assert_equal c1.red,   c2.red
        assert_equal c1.green, c2.green
        assert_equal c1.blue,  c2.blue
        assert_equal 255,      c2.alpha
      end
    end
    texture.save("images/saved_image2.png", nil)
    assert FileTest.file?("images/saved_image2.png")
    texture2 = Texture.load("images/saved_image2.png")
    assert_equal texture.size, texture2.size
    texture.height.times do |j|
      texture.width.times do |i|
        c1 = texture.get_pixel(i, j)
        c2 = texture2.get_pixel(i, j)
        assert_equal c1.red,   c2.red
        assert_equal c1.green, c2.green
        assert_equal c1.blue,  c2.blue
        assert_equal 255,      c2.alpha
      end
    end
    if FileTest.exist?("images/saved_image.png")
      File.delete("images/saved_image.png")
    end
    if FileTest.exist?("images/saved_image2.png")
      File.delete("images/saved_image2.png")
    end
  end
  
  def test_dump
    texture = Texture.load("images/ruby")
    str = texture.dump("rgb")
    assert_equal texture.width * texture.height * 3, str.length
    texture.height.times do |j|
      texture.width.times do |i|
        p = texture.get_pixel(i, j)
        origin = i + j * texture.width
        assert_equal str[3 * origin].ord,     p.red
        assert_equal str[3 * origin + 1].ord, p.green
        assert_equal str[3 * origin + 2].ord, p.blue
      end
    end
    str = texture.dump("rgba")
    assert_equal texture.width * texture.height * 4, str.length
    texture.height.times do |j|
      texture.width.times do |i|
        p = texture.get_pixel(i, j)
        origin = i + j * texture.width
        assert_equal str[4 * origin].ord,     p.red
        assert_equal str[4 * origin + 1].ord, p.green
        assert_equal str[4 * origin + 2].ord, p.blue
        assert_equal str[4 * origin + 3].ord, p.alpha
      end
    end
    str = texture.dump("argb")
    assert_equal texture.width * texture.height * 4, str.length
    texture.height.times do |j|
      texture.width.times do |i|
        p = texture.get_pixel(i, j)
        origin = i + j * texture.width
        assert_equal str[4 * origin].ord,     p.alpha
        assert_equal str[4 * origin + 1].ord, p.red
        assert_equal str[4 * origin + 2].ord, p.green
        assert_equal str[4 * origin + 3].ord, p.blue
      end
    end
    str = texture.dump("rrrragb")
    assert_equal texture.width * texture.height * 7, str.length
    texture.height.times do |j|
      texture.width.times do |i|
        p = texture.get_pixel(i, j)
        origin = i + j * texture.width
        assert_equal str[7 * origin].ord,     p.red
        assert_equal str[7 * origin + 1].ord, p.red
        assert_equal str[7 * origin + 2].ord, p.red
        assert_equal str[7 * origin + 3].ord, p.red
        assert_equal str[7 * origin + 4].ord, p.alpha
        assert_equal str[7 * origin + 5].ord, p.green
        assert_equal str[7 * origin + 6].ord, p.blue
      end
    end
  end
  
  def test_dump_disposed
    texture = Texture.load("images/ruby")
    texture.dispose
    assert_raise RuntimeError do
      texture.dump("rgb")
    end
  end
  
  def test_dump_type
    texture = Texture.load("images/ruby")
    assert_raise TypeError do
      texture.dump(nil)
    end
  end
  
  def test_undump
    orig_texture = Texture.load("images/ruby")
    texture = Texture.new(orig_texture.width, orig_texture.height)
    texture.undump(orig_texture.dump("rgb"), "rgb")
    texture.height.times do |j|
      texture.width.times do |i|
        p1 = texture.get_pixel(i, j)
        p2 = orig_texture.get_pixel(i, j)
        assert_equal p2.red,   p1.red
        assert_equal p2.green, p1.green
        assert_equal p2.blue,  p1.blue
        assert_equal 0,        p1.alpha
      end
    end
    texture.undump(orig_texture.dump("rgba"), "rgba")
    texture.height.times do |j|
      texture.width.times do |i|
        p1 = texture.get_pixel(i, j)
        p2 = orig_texture.get_pixel(i, j)
        assert_equal p2.red,   p1.red
        assert_equal p2.green, p1.green
        assert_equal p2.blue,  p1.blue
        assert_equal p2.alpha, p1.alpha
      end
    end
    texture.undump(orig_texture.dump("argb"), "rgba")
    texture.height.times do |j|
      texture.width.times do |i|
        p1 = texture.get_pixel(i, j)
        p2 = orig_texture.get_pixel(i, j)
        assert_equal p2.red,   p1.green
        assert_equal p2.green, p1.blue
        assert_equal p2.blue,  p1.alpha
        assert_equal p2.alpha, p1.red
      end
    end
    texture.undump("\x12\x34\x56\x78" * texture.width * texture.height, "rgba")
    texture.height.times do |j|
      texture.width.times do |i|
        p1 = texture.get_pixel(i, j)
        assert_equal 0x12, p1.red
        assert_equal 0x34, p1.green
        assert_equal 0x56, p1.blue
        assert_equal 0x78, p1.alpha
      end
    end
    begin
      data = ("\x12\x34\x56\x78" * texture.width * texture.height)[0..-2]
      texture.undump(data, "rgba")
      flunk
    rescue ArgumentError => e
      assert_equal "invalid data size: #{data.size + 1} expected but was #{data.size}", e.message
    end
    begin
      data = "\x12\x34\x56\x78" * texture.width * texture.height + "\0"
      texture.undump(data, "rgba")
      flunk
    rescue ArgumentError => e
      assert_equal "invalid data size: #{data.size - 1} expected but was #{data.size}", e.message
    end
  end
  
  def test_undump_frozen
    texture = Texture.load("images/ruby")
    texture.freeze
    assert_raise FrozenError do
      texture.undump("\x12\x34\x56\x78" * texture.width * texture.height, "rgba")
    end
  end
  
  def test_undump_disposed
    texture = Texture.load("images/ruby")
    texture.dispose
    assert_raise RuntimeError do
      texture.undump("\x12\x34\x56\x78" * texture.width * texture.height, "rgba")
    end
  end
  
  def test_undump_type
    texture = Texture.load("images/ruby")
    assert_raise TypeError do
      texture.undump(nil, "rgba")
    end
    assert_raise TypeError do
      texture.undump("\x12\x34\x56\x78" * texture.width * texture.height, nil)
    end
  end
  
end

class InputTest < Test::Unit::TestCase

  def test_keys_type
    assert_raise TypeError do
      Input.keys(nil)
    end
    begin
      Input.keys(:foo)
      flunk
    rescue ArgumentError => e
      assert_equal "invalid device: :foo", e.message
    end
    assert_raise TypeError do
      Input.keys(:gamepad, false)
    end
    [:device_number, :duration, :delay, :interval].each do |key|
      assert_raise TypeError do
        Input.keys(:gamepad, key => false)
      end
    end
  end
  
  def test_mouse_location
    assert_kind_of Array, Input.mouse_location
    assert_equal 2, Input.mouse_location.size
    assert Input.mouse_location.frozen?
  end
  
  def test_gamepad_device_number
    assert_equal [], Input.keys(:gamepad, :device_number => -1)
    assert_equal [], Input.keys(:gamepad, :device_number => 100)
  end

end

class AudioTest < Test::Unit::TestCase

  def test_audio_type
    assert_raise TypeError do
      Audio.bgm_volume = nil
    end
    assert_raise TypeError do
      Audio.play_bgm(nil)
    end
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
    Audio.bgm_volume = -100
    assert_equal 0, Audio.bgm_volume
    Audio.bgm_volume = 10000
    assert_equal 255, Audio.bgm_volume
  end
  
end
