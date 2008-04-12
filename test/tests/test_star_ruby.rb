#!/usr/bin/env ruby

require "starruby"

class TestStarRuby < Test::Unit::TestCase

  def test_version
    assert_equal "0.1.17", StarRuby::VERSION
  end

end
