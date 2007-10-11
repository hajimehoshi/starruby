require "starruby"
require "test/unit"

include StarRuby

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
  
end
