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
  
end

class ToneTest < Test::Unit::TestCase
  
  def test_tone
    t1 = Tone.new(1, 2, 3, 4)
    t2 = Tone.new(5, 6, 7)

    assert_equal 1, t1.red
    assert_equal 2, t1.green
    assert_equal 3, t1.blue
    assert_equal 4, t1.saturation
    assert_equal 255, t2.saturation

    assert t1 == Tone.new(1, 2, 3, 4)
    assert t2 == Tone.new(5, 6, 7)
    assert t2 == Tone.new(5, 6, 7, 255)
    assert t1 != Tone.new(1, 2, 3, 5)
    assert t1 != Object.new
    assert t1.eql?(Tone.new(1, 2, 3, 4))
    assert_equal t1.hash, Tone.new(1, 2, 3, 4).hash
    assert_equal t2.hash, Tone.new(5, 6, 7).hash
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
  
  #def test_offscreen
    #assert_kind_of Texture, Screen.offscreen
    #assert_equal Screen.width, Screen.offscreen.width
    #assert_equal Screen.height, Screen.offscreen.height
  #end
  
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
  end
  
  def test_dup
    texture = Texture.load("images/ruby")
    texture2 = texture.dup
    assert_equal texture.size, texture2.size
    texture.freeze
    assert !texture.dup.frozen?
  end
  
  def test_dispose
    texture = Texture.load("images/ruby")
    assert_equal false, texture.disposed?
    texture.dispose
    assert_equal true, texture.disposed?
    assert_raise StarRubyError do
      texture.dispose
    end
    # change_hue
  end
  
  def test_get_and_set_pixel
    texture = Texture.new(3, 3)
    texture.height.times do |y|
      texture.width.times do |x|
        assert_equal Color.new(0, 0, 0, 0), texture.get_pixel(x,y)
      end
    end
=begin
    assert_raise IndexError do
      texture.get_pixel 3, 2
    end
    
    texture.set_pixel 0, 1, Color.new(31, 41, 59, 26)
    texture.set_pixel 1, 2, Color.new(53, 58, 97, 92)
    texture.set_pixel 2, 0, Color.new(65, 35, 89, 79)
    assert_equal Color.new(31, 41, 59, 26), texture.get_pixel(0, 1);
    assert_equal Color.new(53, 58, 97, 92), texture.get_pixel(1, 2);
    assert_equal Color.new(65, 35, 89, 79), texture.get_pixel(2, 0);
    
    assert_raise IndexError do
      texture.get_pixel 2, 3
    end
=end
  end
  
end
