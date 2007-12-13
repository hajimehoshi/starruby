#!/usr/bin/env ruby

require "test/unit"

GC.stress = true if GC.respond_to?(:stress=)

require "starruby"
include StarRuby

class GCStressTest < Test::Unit::TestCase
  
  def test_color
    color = Color.new(1, 2, 3, 4)
    color.red
    color.green
    color.blue
    color.alpha
    color == Color.new(1, 2, 3, 4)
    color.hash
    color.eql?(Color.new(1, 2, 3, 4))
    color.to_s
  end
  
  def test_font
    Font.exist?("Arial")
    Font.exist?("FreeSans")
    Font.exist?("fonts/ORANGEKI")
    font = Font.new("fonts/ORANGEKI", 12, {
      :bold => true, :italic => true, :ttc_index => 0
    })
    font.name
    font.bold?
    font.italic?
    font.size
  end
  
end

