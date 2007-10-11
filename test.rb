require "starruby"
require "test/unit"

include StarRuby

class GameTest < Test::Unit::TestCase
  
  def test_run
    Game.run do
    end
  end
  
end
