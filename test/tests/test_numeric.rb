#!/usr/bin/env ruby

require "test/unit"
require "starruby"

class TestNumeric < Test::Unit::TestCase

  def test_degree
    -10.step(370, 0.25) do |i|
      assert_equal i * 2 * Math::PI / 360, i.degree
      assert_equal i * 2 * Math::PI / 360, i.degrees
    end
  end

end
