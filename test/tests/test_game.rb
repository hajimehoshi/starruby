#!/usr/bin/env ruby

require "starruby"
include StarRuby

class TestGame < Test::Unit::TestCase

  def test_new_defaults
    g = nil
    begin
      g = Game.new(320, 240)
      assert_equal "", g.title
      assert_equal 30, g.fps
      assert_equal 1, g.window_scale
      assert_equal false, g.disposed?
    ensure
      if g
        g.dispose
        assert_equal true, g.disposed?
      end
    end
  end

  def test_new_duplicate
    g = nil
    begin
      g = Game.new(320, 240)
      assert_raise StarRubyError do
        Game.new(320, 240)
      end
    ensure
      g.dispose if g
    end
  end

  def test_new_with_options
    assert_nil Game.current
    g = nil
    begin
      g = Game.new(320, 240,
                   :title => "foo", :fps => 31, :window_scale => 2)
      assert_equal Game.current, g
      assert_equal false, g.window_closing?
      assert_not_nil g.screen
      assert_equal "foo", g.title
      title = g.title = "bar"
      assert_equal "bar", g.title
      title << "baz"
      assert_equal "bar", g.title
      assert_equal 31, g.fps
      g.fps = 32
      assert_equal 32, g.fps
      assert_equal 2, g.window_scale
      assert_equal 0.0, g.real_fps
      assert_kind_of Float, g.real_fps
      g.update_state
      g.update_screen
      g.update_state
      g.update_screen
    ensure
      g.dispose if g
    end
    assert_nil Game.current
  end

  def test_new_type
    assert_raise TypeError do
      Game.new(nil, 240)
    end
    assert_raise TypeError do
      Game.new(320, nil)
    end
    assert_raise TypeError do
      Game.new(320, 240, false)
    end
    assert_raise TypeError do
      Game.new(320, 240, :title => false)
    end
    assert_raise TypeError do
      Game.new(320, 240, :fps => false)
    end
    assert_raise TypeError do
      Game.new(320, 240, :window_scale => false)
    end
  end

  def test_dispose
    g = Game.new(320, 240)
    assert_equal false, g.disposed?
    g.dispose
    assert_equal true, g.disposed?
    assert_raise RuntimeError do
      g.fps
    end
    assert_raise RuntimeError do
      g.fps = 30
    end
    assert_raise RuntimeError do
      g.real_fps
    end
    assert_raise RuntimeError do
      g.screen
    end
    assert_raise RuntimeError do
      g.title
    end
    assert_raise RuntimeError do
      g.title = "foo"
    end
    assert_raise RuntimeError do
      g.update_screen
    end
    assert_raise RuntimeError do
      g.update_state
    end
    assert_raise RuntimeError do
      g.wait
    end
    assert_raise RuntimeError do
      g.window_closing?
    end
    assert_raise RuntimeError do
      g.window_scale
    end
  end

  def test_run
    assert_nil Game.current
    called = false
    Game.run(320, 240, :title => "foo") do |g|
      called = true
      assert_not_nil Game.current
      assert_equal "foo", g.title
      assert_equal 30, g.fps
      assert_raise StarRubyError do
        Game.run(320, 240) {}
      end
      break
    end
    assert called
    assert_nil Game.current
  end

  def test_run_error
    assert_nil Game.current
    begin
      Game.run(320, 240) do |game|
        assert_not_nil game.screen
        raise RuntimeError, "runtime error"
      end
    rescue RuntimeError
    end
    assert_nil Game.current
  end

  def test_run_window_scale
    Game.run(320, 240, :window_scale => 2) do |game|
      assert_equal [320, 240], game.screen.size
      break
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
      Game.run(320, 240, :title => false) {}
    end
    assert_raise TypeError do
      Game.run(320, 240, :fps => false) {}
    end
    assert_raise TypeError do
      Game.run(320, 240, :window_scale => false) {}
    end
  end

  def test_screen
    Game.run(320, 240) do |game|
      begin
        assert_kind_of Texture, game.screen
        assert_equal [320, 240], game.screen.size
      ensure
        break
      end
    end
    Game.run(123, 456) do |game|
      begin
        assert_kind_of Texture, game.screen
        assert_equal [123, 456], game.screen.size
      ensure
        break
      end
    end
  end

  def test_ticks
    ticks1 = Game.ticks
    ticks2 = 0
    Game.run(320, 240) do
      ticks2 = Game.ticks
      assert ticks1 <= ticks2
      break
    end
    ticks3 = Game.ticks
    assert ticks2 <= ticks3
  end

end
