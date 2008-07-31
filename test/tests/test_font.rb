#!/usr/bin/env ruby

require "starruby"
include StarRuby

class TestFont < Test::Unit::TestCase

  def test_exist
    case RUBY_PLATFORM
    when /mswin32|cygwin|mingw32|bccwin32|interix|djgpp/
      # Windows
      assert_equal true,  Font.exist?("Arial")
      assert_equal false, Font.exist?("arial.ttf")
      assert_equal false, Font.exist?("arial.ttc")
      assert_equal (File.exist?("./arial.ttf") or File.exist?("./arial.ttc")), Font.exist?("arial")
      assert_equal (File.exist?("./Arial.ttf") or File.exist?("./Arial.ttc")), Font.exist?("./Arial")
      assert_equal (File.exist?("./arial.ttf") or File.exist?("./arial.ttc")), Font.exist?("./arial")
      assert_equal false, Font.exist?("msgothic")
      assert_equal false, Font.exist?("msgothic.ttf")
      assert_equal false, Font.exist?("msgothic.ttc")
      assert_equal true,  Font.exist?("Arial")
      assert_equal true, (Font.exist?("ＭＳ ゴシック") or Font.exist?("MS Gothic"))
      assert_equal false, Font.exist?("notfont")
      assert_equal false, Font.exist?("notfont.ttf")
      assert_equal false, Font.exist?("notfont.ttc")
    when /linux/
      # Linux
      assert_equal true,  Font.exist?("FreeSans")
      assert_equal true,  Font.exist?("FreeSans")
      assert_equal false, Font.exist?("FreeSans:style=Bold")
      assert_equal true,  Font.exist?("FreeSans, Bold")
      assert_equal true,  Font.exist?("FreeSans, BoldOblique")
      assert_equal false, Font.exist?("FreeSans, NotStyle")
      assert_equal true,  Font.exist?("FreeSans ,Bold")
      assert_equal true,  Font.exist?("FreeSans ,")
      assert_equal false, Font.exist?("FreeSans.ttf")
      assert_equal false, Font.exist?("FreeSans.ttc")
    when /darwin/
      # Mac OS X
      assert_equal true,  Font.exist?("Helvetica")
      assert_equal true,  Font.exist?("Helvetica, Regular")
      assert_equal true,  Font.exist?("Helvetica, Oblique")
      assert_equal false, Font.exist?("Helvetica, NotStyle")
    end
    assert_raise ArgumentError do
      Font.exist?("fonts/maybefont")
    end
    assert_equal true, Font.exist?("fonts/ORANGEKI")
    assert_equal true, Font.exist?("fonts/maybefont2")
  end

  def test_exist_type
    assert_raise TypeError do
      Font.exist?(nil)
    end
  end

  def test_new
    if Font.exist?("Arial")
      font = Font.new("Arial", 16)
      assert_equal "Arial", font.name
    elsif Font.exist?("FreeSans")
      font = Font.new("FreeSans", 16)
      assert_equal "FreeSans", font.name
    elsif Font.exist?("Helvetica Neue")
      font = Font.new("Helvetica Neue", 16)
      assert_equal "Helvetica Neue", font.name
    else
      flunk
    end
    assert_equal 16, font.size
    assert_equal false, font.bold?
    assert_equal false, font.italic?
    if Font.exist?("MS UI Gothic")
      font = Font.new("MS UI Gothic", 12, {
                        :ttc_index => 1, :bold => true, :italic => true
                      }) # :ttc_index is ignored
      assert_equal 12, font.size
      assert_equal "MS UI Gothic", font.name
      assert_equal true, font.bold?
      assert_equal true, font.italic?
    end
    assert_raise Errno::ENOENT do
      Font.new("notfont", 12)
    end
    assert_raise ArgumentError do
      Font.new("fonts/maybefont", 12)
    end
    assert_raise StarRubyError do
      Font.new("fonts/maybefont2", 12)
    end
  end

  def test_equal
    case RUBY_PLATFORM
    when /mswin32|cygwin|mingw32|bccwin32|interix|djgpp/
      names = ["MS UI Gothic", "Arial", "Arial Black"]
    when /darwin/
      names = ["Bitstream Charter, Bold", "Bitstream Charter, Bold Italic", "Courier, Regular"]
    when /linux/
      names = ["Bitstream Charter, Bold", "Bitstream Charter, Bold Italic", "Courier 10 Pitch, Regular"]
    end
    assert names.all?{|name| Font.exist?(name)}
    assert Font.new(names[0], 12).equal?(Font.new(names[0], 12))
    assert ! Font.new(names[0], 12).equal?(Font.new(names[1], 12))
    assert ! Font.new(names[0], 12).equal?(Font.new(names[2], 12))
    assert Font.new(names[1], 12).equal?(Font.new(names[1], 12))
    assert ! Font.new(names[1], 12).equal?(Font.new(names[2], 12))
    assert Font.new(names[2], 12).equal?(Font.new(names[2], 12))
    assert ! Font.new(names[0], 12).equal?(Font.new(names[0], 11))
    assert ! Font.new(names[0], 12).equal?(Font.new(names[0], 13))
    assert ! Font.new(names[0], 12).equal?(Font.new(names[0], 12, :bold => true))
    assert ! Font.new(names[0], 12).equal?(Font.new(names[0], 12, :italic => true))
    assert ! Font.new(names[0], 12).equal?(Font.new(names[0], 12, :bold => true, :italic => true))
    assert Font.new(names[0], 12, :bold => true).equal?(Font.new(names[0], 12, :bold => true))
    assert Font.new(names[0], 12, :italic => true).equal?(Font.new(names[0], 12, :italic => true))
    assert Font.new(names[0], 12, :bold => true, :italic => true).equal?(Font.new(names[0], 12, :bold => true, :italic => true))
    assert ! Font.new(names[0], 12).equal?(Font.new(names[1], 14, :bold => true))
    assert Font.new(names[1], 14, :bold => true).equal?(Font.new(names[1], 14, :bold => true))
  end

  def test_new_type
    assert_raise TypeError do
      Font.new(nil, 12)
    end
    assert_raise TypeError do
      Font.new("fonts/ORANGEKI", nil)
    end
    assert_raise TypeError do
      Font.new("fonts/ORANGEKI", 12, false)
    end
  end

  def test_get_size
    if Font.exist?("ＭＳ ゴシック")
      font = Font.new("ＭＳ ゴシック", 12)
      assert_equal "MS Gothic", font.name
      assert_equal [6, 13], font.get_size("A")
      assert font.get_size("A").frozen?
      h = font.get_size("A")[1]
      assert_equal [78, h], font.get_size("Hello, World!")
      assert_equal [60, h], font.get_size("こんにちは")
      assert_equal [30, h], font.get_size("aaa&a")
    end
    if Font.exist?("Arial")
      font = Font.new("Arial", 16)
    elsif Font.exist?("FreeSans")
      font = Font.new("FreeSans", 16)
    elsif Font.exist?("Helvetica Neue")
      font = Font.new("Helvetica Neue", 16)
    else
      flunk
    end
    size = font.get_size("AAAAAAAAAAAAAA");
    size[0] # No Exception
    size[1] # No Exception
  end

  def test_get_size_type
    font = Font.new("fonts/ORANGEKI", 12)
    assert_raise TypeError do
      font.get_size(nil)
    end
  end

end
