require "falling_blocks/piece"
require "falling_blocks/field"

module FallingBlocks
  
  class ApplicationModel
    
    attr_reader :state
    
    attr_reader :score
    def level
      @lines / 5 + 1
    end
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
    
    def initialize
      @state = :start
      @score = 0
      @lines = 0
      @field = Field.new
      go_next_piece
    end
    
    def start_playing
      @state = :playing
    end
    
    def pause
      @state = :pause
    end
    
    def unpause
      @state = :playing if @state == :pause
    end
    
    def try_move(direction)
      x_100 = @falling_piece_x_100
      case direction
      when :left
        x_100 -= 100
      when :right
        x_100 += 100
      end
      x = x_100.quo(100).ceil
      y = @falling_piece_y_100.quo(100).ceil
      angle = @falling_piece_angle
      unless field.conflict?(@falling_piece, x, y, angle)
        @falling_piece_x_100 = x_100
      end
    end
    
    def try_rotate(direction)
      angle = @falling_piece_angle
      case direction
      when :left
        angle = (angle + 3) % 4
      when :right
        angle = (angle + 1) % 4
      end
      x = @falling_piece_x_100.quo(100).ceil
      y = @falling_piece_y_100.quo(100).ceil
      unless field.conflict?(@falling_piece, x, y, angle)
        @falling_piece_angle = angle
      end
    end
    
    def try_fall(down_key = false)
      dy_100 = level * 5
      dy_100 = [dy_100, 100].max if down_key
      y_100 = @falling_piece_y_100
      x = @falling_piece_x_100.quo(100).ceil
      angle = @falling_piece_angle
      loop do
        y_100 += [100, dy_100].min
        dy_100 -= [100, dy_100].min
        y = y_100.quo(100).ceil
        if field.conflict?(@falling_piece, x, y, angle)
          @falling_piece_y_100 = (y - 1) * 100
          break
        end
        if dy_100 <= 0
          @falling_piece_y_100 = y_100
          break
        end
      end
    end
    
    def add_score(d_score)
      @score += d_score
    end
    
    def falling_piece_landing?
      x = @falling_piece_x_100.quo(100).ceil
      y = @falling_piece_y_100.quo(100).floor
      angle = @falling_piece_angle
      field.conflict?(@falling_piece, x, y + 1, angle)
    end
    
    def do_landing
      x = @falling_piece_x_100 / 100
      y = @falling_piece_y_100 / 100
      angle = @falling_piece_angle
      @field.add_piece(@falling_piece, x, y, angle)
      go_next_piece if @field.flashing_lines.empty?
    end
    
    def flashing?
      not @field.flashing_lines.empty?
    end
    
    def finish_flashing
      lines_size = @field.flashing_lines.size
      @score += (case lines_size
      when 1; 100
      when 2; 300
      when 3; 900
      when 4; 2700
      end) * level
      @lines += lines_size
      @field.flash
      go_next_piece
    end
    
    private
    
    def go_next_piece
      @falling_piece = @next_piece || Piece[rand(Piece.count)]
      @next_piece = Piece[rand(Piece.count)]
      @falling_piece_x_100 = (field.width - @falling_piece.width) / 2 * 100
      @falling_piece_y_100 = 0
      @falling_piece_angle = 0
    end
    
  end
  
end
