module FallingBlocks
  
  class Piece

    def initialize(number, width, blocks)
      @number = number
      @width = width
      @blocks = blocks
    end
    
    @@pieces = []
    @@pieces << Piece.new(0, 4, [1, 1, 1, 1])
    @@pieces << Piece.new(1, 3, [1, 0, 0, 1, 1, 1])
    @@pieces << Piece.new(2, 3, [0, 1, 0, 1, 1, 1])
    @@pieces << Piece.new(3, 3, [0, 0, 1, 1, 1, 1])
    @@pieces << Piece.new(4, 3, [1, 1, 0, 0, 1, 1])
    @@pieces << Piece.new(5, 3, [0, 1, 1, 1, 1, 0])
    @@pieces << Piece.new(6, 2, [1, 1, 1, 1])
    
    def self.[](number)
      @@pieces[number]
    end

    def [](x, y, angle = 0)

    end

  end
  
end
