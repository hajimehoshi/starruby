#!/usr/bin/env ruby

require "starruby"
include StarRuby

class TestColor < Test::Unit::TestCase

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
    assert_raise ArgumentError do
      Color.new(-1, 0, 0, 0)
    end
    assert_raise ArgumentError do
      Color.new(256, 0, 0, 0)
    end
    assert_raise ArgumentError do
      Color.new(0, -1, 0, 0)
    end
    assert_raise ArgumentError do
      Color.new(0, 256, 0, 0)
    end
    assert_raise ArgumentError do
      Color.new(0, 0, -1, 0)
    end
    assert_raise ArgumentError do
      Color.new(0, 0, 256, 0)
    end
    assert_raise ArgumentError do
      Color.new(0, 0, 0, -1)
    end
    assert_raise ArgumentError do
      Color.new(0, 0, 0, 256)
    end
  end

  def test_to_s
    c = Color.new(1, 2, 3, 4)
    assert_equal "#<StarRuby::Color alpha=4, red=1, green=2, blue=3>", c.to_s
    c = Color.new(255, 255, 255, 255)
    assert_equal "#<StarRuby::Color alpha=255, red=255, green=255, blue=255>", c.to_s
  end

end
