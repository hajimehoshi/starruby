include StarRuby

module FallingBlocks
  
  class View
    
    def initialize
      @textures = {
        :background => Texture.load("images/falling_blocks/background"),
        :field_window => Texture.new(100, 200),
        :next_piece_window => Texture.new(50, 50),
        :score_window => Texture.new(140, 20),
        :level_window => Texture.new(140, 20),
        :lines_window => Texture.new(140, 20),
      }
      @font = Font.new("fonts/falling_blocks/flappy_for_famicom", 8)
    end
    
    def render_text(screen, text, x, y, in_window = false)
      fore_color = in_window ? Color.new(255, 255, 255) : Color.new(51, 51, 153)
      screen.render_text(text, x + 1, y + 8 + 1, @font, Color.new(0, 0, 0, 64))
      screen.render_text(text, x, y + 8, @font, fore_color)
    end
    
    private :render_text
    
    def update(game, screen)
      # clear windows
      @textures.keys.select{|k| k.to_s =~ /window$/}.each do |key|
        @textures[key].fill(Color.new(0, 0, 0, 192))
      end
      
      # render field
      texture = @textures[:field_window]
      field = game.field
      field.height.times do |j|
        field.width.times do |i|
          if block = field[i, j]
            # texture.render_texture()
          end
        end
      end
      
      # render texts
      %w(score level lines).each do |key|
        value = game.send(key).to_s
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
    end
    
  end
  
end
