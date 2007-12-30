module FallingBlocks
  
  class Piece
    
    attr_reader :id
    attr_reader :width
    alias height width

    def initialize(id, width, blocks)
      @id = id
      @width = width
      @blocks = blocks
    end
    
    @@pieces = []
    @@pieces << Piece.new(0, 4, [0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0].map{|n| n != 0})
    @@pieces << Piece.new(1, 3, [1,0,0, 1,1,1, 0,0,0].map{|n| n != 0})
    @@pieces << Piece.new(2, 3, [0,1,0, 1,1,1, 0,0,0].map{|n| n != 0})
    @@pieces << Piece.new(3, 3, [0,0,1, 1,1,1, 0,0,0].map{|n| n != 0})
    @@pieces << Piece.new(4, 3, [1,1,0, 0,1,1, 0,0,0].map{|n| n != 0})
    @@pieces << Piece.new(5, 3, [0,1,1, 1,1,0, 0,0,0].map{|n| n != 0})
    @@pieces << Piece.new(6, 2, [1,1,1,1].map{|n| n != 0})
    
    def self.[](id)
      @@pieces[id]
    end
    
    def self.count
      @@pieces.size
    end

    def [](x, y, angle = 0)
      case angle
      when 0
        @blocks[x + y * width]
      when 1
        @blocks[y + (width - x - 1) * width]        
      when 2
        @blocks[(width - x - 1) + (width - y - 1) * width]
      when 3
        @blocks[(width - y - 1) + x * width]
      end
    end

  end
  
end
