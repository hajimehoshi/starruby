#!/usr/bin/env ruby

if RUBY_VERSION < "1.9.0"
  require "test/unit"
  Test::Unit::AutoRunner.run(true, "./tests")
else
  require "minitest/unit"
  u = MiniTest::Unit
  def u.autorun
    # do nothing
  end
  require "test/unit"
  Dir.glob("./tests/*.rb") do |path|
    require(path)
  end
  MiniTest::Unit.new.run
end
