#!/usr/bin/env ruby

require "test/unit"

if defined? Test::Unit::AutoRunner
  Test::Unit::AutoRunner.run(true, "./tests")
else
  Dir.glob("./tests/*.rb") do |path|
    require(path)
  end
end
