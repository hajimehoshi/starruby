module FallingBlocks
  
  class Piece
    
    attr_reader :id

    def initialize(id, blocks)
      @id = id
      @blocks = blocks
    end

    t = true
    _ = false
    @@pieces = []
    @@pieces << Piece.new(0, [_,_,_,_,
                              t,t,t,t,
                              _,_,_,_,
                              _,_,_,_])
    @@pieces << Piece.new(1, [t,_,_,
                              t,t,t,
                              _,_,_])
    @@pieces << Piece.new(2, [_,t,_,
                              t,t,t,
                              _,_,_])
    @@pieces << Piece.new(3, [_,_,t,
                              t,t,t,
                              _,_,_])
    @@pieces << Piece.new(4, [t,t,_,
                              _,t,t,
                              _,_,_])
    @@pieces << Piece.new(5, [_,t,t,
                              t,t,_,
                              _,_,_])
    @@pieces << Piece.new(6, [t,t,
                              t,t])
    
    def self.[](id)
      @@pieces[id]
    end
    
    def self.count
      @@pieces.size
    end

    def width
      @width ||= Math.sqrt(@blocks.size).to_i
    end

    alias height width

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
