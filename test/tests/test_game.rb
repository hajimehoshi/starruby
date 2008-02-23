#!/usr/bin/env ruby

require "starruby"
include StarRuby

class TestGame < Test::Unit::TestCase

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
  
  def test_cursor_visible
    assert_equal false, Game.cursor_visible?
    Game.cursor_visible = true
    assert_equal true,  Game.cursor_visible?
    Game.cursor_visible = false
    assert_equal false, Game.cursor_visible?
    Game.run(320, 240) do
      Game.cursor_visible = true
      assert_equal true,  Game.cursor_visible?
      Game.cursor_visible = false
      assert_equal false, Game.cursor_visible?
      Game.terminate
    end
    Game.cursor_visible = true
    assert_equal true,  Game.cursor_visible?
    Game.cursor_visible = false
    assert_equal false, Game.cursor_visible?
  end

end
