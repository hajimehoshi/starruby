require "falling_blocks/piece"
require "falling_blocks/field"

module FallingBlocks
  
  class Game
    
    attr_reader :score
    attr_reader :level
    attr_reader :lines
    attr_reader :field
    
    def initialize
      @score = 0
      @level = 1
      @lines = 0
      @field = Field.new
    end
    
    def next_piece
    end
    
    def update
      
    end
    
  end
  
end
