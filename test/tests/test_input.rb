#!/usr/bin/env ruby

require "starruby"
include StarRuby

class TestInput < Test::Unit::TestCase

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

  def test_mouse_location_eq
    Input.mouse_location = 1, 2
    assert_raise ArgumentError do
      Input.mouse_location = 1, 2, 3
    end
    assert_raise TypeError do
      Input.mouse_location = false
    end
  end
  
  def test_gamepad_device_number
    assert_equal [], Input.keys(:gamepad, :device_number => -1)
    assert_equal [], Input.keys(:gamepad, :device_number => 100)
  end

end
