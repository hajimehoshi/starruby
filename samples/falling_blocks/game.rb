require "falling_blocks/piece"
require "falling_blocks/field"

module FallingBlocks
  
  class Game
    
    attr_reader :state
    
    attr_reader :score
    attr_reader :level
    attr_reader :lines
    attr_reader :field
    
    attr_reader :falling_piece
    def falling_piece_x
      @falling_piece_x_100.quo(100)
    end
    def falling_piece_y
      @falling_piece_y_100.quo(100)
    end
    attr_reader :falling_piece_angle
    
    attr_reader :next_piece
    
    def update
      @score ||= 0
      @level ||= 1
      @lines ||= 0
      @field ||= Field.new
      @state ||= :start
      case @state
      when :start
        if Input.pressed_keys(:keyboard, :duration => 1).include?(:enter)
          @state = :playing
        end
      when :playing
        @next_piece ||= Piece[rand(Piece.count)]
        @falling_piece ||= Piece[rand(Piece.count)]
        @falling_piece_x_100 ||= (field.width - @falling_piece.width) / 2 * 100
        @falling_piece_y_100 ||= 0
        @falling_piece_angle ||= 0
        
        fp_x = @falling_piece_x_100
        fp_y = @falling_piece_y_100
        fp_angle = @falling_piece_angle
        
        keys_repeating = Input.pressed_keys(:keyboard, {
          :duration => 1, :delay => 2, :interval => 0
        })
        if keys_repeating.include?(:left)
          fp_x -= 100
        elsif keys_repeating.include?(:right)
          fp_x += 100
        end
        
        bps = @level
        if Input.pressed_keys(:keyboard).include?(:down)
          bps = [bps, 100].max
          @score += 1
        end
        fp_y += bps
        
        if Input.pressed_keys(:keyboard, :duration => 1).include?(:z)
          fp_angle = (fp_angle + 3) % 4
        elsif Input.pressed_keys(:keyboard, :duration => 1).include?(:x)
          fp_angle = (fp_angle + 1) % 4
        end
        
        @falling_piece_x_100 = fp_x
        @falling_piece_y_100 = fp_y
        @falling_piece_angle = fp_angle
      end
    end
    
  end
  
end
