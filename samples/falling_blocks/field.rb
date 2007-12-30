module FallingBlocks
  
  class Field
    
    def initialize
      @blocks = Array.new(width * height){nil}
    end
    
    def width
      10
    end
    
    def height
      20
    end
    
    def [](x, y)
      @blocks[x + y * width]
    end
    
    def add_piece(piece, x, y)
    end
    
  end
  
end
