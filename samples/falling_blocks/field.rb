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
      return true unless (0...width).include?(x) and (0...height).include?(y)
      @blocks[x + y * width]
    end
    
    def conflict?(piece, x, y, angle)
      piece.height.times do |j|
        piece.width.times do |i|
          return true if piece[i, j, angle] and self[x + i, y + j]
        end
      end
      false
    end
    
    def add_piece(piece, x, y, angle)
      piece.height.times do |j|
        piece.width.times do |i|
          @blocks[(x + i) + (y + j) * width] = piece.id if piece[i, j, angle]
        end
      end
    end
    
    def flashing_lines
      (0...height).select do |j|
        @blocks[j * width, width].all?
      end
    end
    
    def flash
      j = height - 1
      while 0 <= j
        if @blocks[j * width, width].all?
          j.downto(1) do |j2|
            width.times do |i|
              @blocks[i + j2 * width] = @blocks[i + (j2 - 1) * width]
            end
          end
        else
          j -= 1
        end
      end
    end
    
  end
  
end
