#!/usr/bin/env ruby

require "test/unit"
require "starruby"

class TestStarRuby < Test::Unit::TestCase

  def test_version
    assert_equal "0.3.1", StarRuby::VERSION
    assert StarRuby::VERSION.frozen?
  end

end
