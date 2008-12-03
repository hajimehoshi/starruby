include StarRuby

module FallingBlocks
  
  class View
    
    def render_text(screen, text, x, y, in_window = false)
      fore_color = in_window ? Color.new(255, 255, 255) : Color.new(51, 51, 153)
      screen.render_text(text, x + 1, y + 8 + 1, @font, Color.new(0, 0, 0, 64))
      screen.render_text(text, x, y + 8, @font, fore_color)
    end
    
    private :render_text
    
    def render_piece(screen, piece, x, y, angle = 0, options = {})
      blocks = @textures[:blocks]
      piece.height.times do |j|
        piece.width.times do |i|
          if piece[i, j, angle]
            screen.render_texture(blocks, x + i * 10, y + j * 10, {
              :src_x => piece.id * 10, :src_width => 10, :src_height => 10
            }.merge(options))
          end
        end
      end
    end
    
    private :render_piece

    attr_reader :model
    
    def initialize(model)
      @model = model
      @textures = {
        :background        => Texture.load("images/falling_blocks/background"),
        :blocks            => Texture.load("images/falling_blocks/blocks"),
        :field_window      => Texture.new(100, 200),
        :next_piece_window => Texture.new(50, 50),
        :score_window      => Texture.new(140, 20),
        :level_window      => Texture.new(140, 20),
        :lines_window      => Texture.new(140, 20),
        :start_info        => Texture.new(320, 240),
        :pause_info        => Texture.new(320, 240),
        :gameover_info     => Texture.new(320, 240),
      }
      @font = Font.new("fonts/falling_blocks/flappy_for_famicom", 8)
      
      texture = @textures[:start_info]
      texture.fill(Color.new(0, 0, 0, 128))
      str = "PRESS ANY KEY TO PLAY"
      width, height = @font.get_size(str)
      render_text(texture, str, (texture.width - width) / 2, (texture.height - height) / 2, true)
      
      texture = @textures[:pause_info]
      texture.fill(Color.new(0, 0, 0, 128))
      str = "PAUSE"
      width, height = @font.get_size(str)
      render_text(texture, str, (texture.width - width) / 2, (texture.height - height) / 2, true)
      
      texture = @textures[:gameover_info]
      texture.fill(Color.new(0, 0, 0, 128))
      str = "GAME OVER"
      width, height = @font.get_size(str)
      render_text(texture, str, (texture.width - width) / 2, (texture.height - height) / 2, true)
    end
    
    def update(screen)
      # Clear windows
      @textures.keys.select{|k| k.to_s =~ /window$/}.each do |key|
        @textures[key].fill(Color.new(0, 0, 0, 192))
      end
      
      # Render the field
      if [:playing, :gameover].include?(model.state)
      window = @textures[:field_window]
        blocks = @textures[:blocks]
        field = model.field
        field.height.times do |j|
          field.width.times do |i|
            if field[i, j]
              window.render_texture(blocks, i * 10, j * 10, {
                :src_x => field[i, j] * 10, :src_width => 10, :src_height => 10
              })
            end
          end
        end
      end
      
      # Render the falling piece
      if [:playing, :gameover].include?(model.state)
        window = @textures[:field_window]
        x = model.falling_piece_x
        y = model.falling_piece_y
        angle = model.falling_piece_angle
        options = {}
        options.merge!({
          :tone_red => 128, :tone_green => 128, :tone_blue => 128
        }) if model.falling_piece_landing?
        render_piece(window, model.falling_piece, x * 10, y * 10, angle, options)
      end
      
      # Render flashing
      if model.state == :playing and model.flashing?
        window = @textures[:field_window]
        lines = model.field.flashing_lines
        flashing_texture = Texture.new(model.field.width * 10, 10)
        flashing_texture.fill(Color.new(255, 0, 0, 128))
        lines.each do |line|
          window.render_texture(flashing_texture, 0, line * 10)
        end
      end
      
      # Render the next piece
      if [:playing, :gameover].include?(model.state) and model.next_piece
        window = @textures[:next_piece_window]
        x = (window.width - model.next_piece.width * 10) / 2
        y = (window.height - model.next_piece.height * 10) / 2
        render_piece(window, model.next_piece, x, y)
      end
      
      # Render texts
      %w(score level lines).each do |key|
        value = model.send(key).to_s
        texture = @textures["#{key}_window".intern]
        x = texture.width - @font.get_size(value)[0] - 5
        render_text(texture, value, x, 0, true)
      end
      
      screen.clear
      
      screen.render_texture(@textures[:background], 0, 0)
      screen.render_texture(@textures[:field_window], 20, 20)
      
      render_text(screen, "NEXT", 140, 15)
      screen.render_texture(@textures[:next_piece_window], 140, 35)
      render_text(screen, "SCORE", 140, 100)
      screen.render_texture(@textures[:score_window], 140, 120)
      render_text(screen, "LEVEL", 140, 140)
      screen.render_texture(@textures[:level_window], 140, 160)
      render_text(screen, "LINES", 140, 180)
      screen.render_texture(@textures[:lines_window], 140, 200)
      
      case model.state
      when :start
        screen.render_texture(@textures[:start_info], 0, 0)
      when :pause
        screen.render_texture(@textures[:pause_info], 0, 0)
      when :gameover
        screen.render_texture(@textures[:gameover_info], 0, 0)
      end
    end
    
  end
  
end
