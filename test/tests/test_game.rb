#!/usr/bin/env ruby

require "starruby"
include StarRuby

class TestGame < Test::Unit::TestCase

  def test_game
    assert_equal false, Game.running?
    called = false
    Game.run(320, 240, :title => "foo") do |g|
      called = true
      assert_equal true, Game.running?
      assert_equal "foo", g.title
      assert_equal 30, g.fps
      assert_raise StarRubyError do
        Game.run(320, 240) {}
      end
      Game.terminate
      assert_equal true, Game.running?
    end
    assert called
    assert_equal false, Game.running?
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

  def test_ticks
    ticks1 = Game.ticks
    ticks2 = 0
    Game.run(320, 240) do
      ticks2 = Game.ticks
      assert ticks1 <= ticks2
      Game.terminate
    end
    ticks3 = Game.ticks
    assert ticks2 <= ticks3
  end

  def test_new_defaults
    g = nil
    begin
      g = Game.new(320, 240)
      assert_equal "", g.title
      assert_equal 30, g.fps
    ensure
      g.dispose if g
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
      g = Game.new(320, 240, :title => "foo", :fps => 31)
      assert_equal Game.current, g
      assert_not_nil g.screen
      assert_equal Game.screen, g.screen
      assert_equal "foo", g.title
      g.title = "bar"
      assert_equal "bar", g.title
      assert_equal 31, g.fps
      g.fps = 32
      assert_equal 32, g.fps
      assert_equal false, g.terminated?
      g.update
      g.update
      g.update
      g.terminate
      assert_equal true, g.terminated?
    ensure
      g.dispose if g
    end
    assert_nil Game.current
  end

end
