require "starruby"

include StarRuby

if Font.exist?("MS UI Gothic")
  font = Font.new("MS UI Gothic", 12)
elsif Font.exist?("FreeSans:style=Medium")
  font = Font.new("FreeSans:style=Medium", 12)
end
color = Color.new(255, 255, 255)

class Sprite
  
  TEXTURE = Texture.load "images/Sprite"
  MAX_X = 320 - TEXTURE.width
  MAX_Y = 240 - TEXTURE.height
  
  attr_reader :x
  attr_reader :y
  
  def initialize
    @x = rand(MAX_X)
    @y = rand(MAX_Y)
    @vx = rand(2) * 2 - 1
    @vy = rand(2) * 2 - 1
  end
  
  def update
    @x += @vx
    @y += @vy
    if @x < 0
      @x = -@x
      @vx = 1
    end
    if @y < 0
      @y = -@y
      @vy = 1
    end
    if MAX_X <= @x
      @x = -(@x - MAX_X) + MAX_X
      @vx = -1
    end
    if MAX_Y <= @y
      @y = -(@y - MAX_Y) + MAX_Y
      @vy = -1
    end
  end
  
end


sprites = Array.new(1000) { Sprite.new }

i = 0
bgm_position = 0

#Game.fps = 10000
Game.run(320, 240, :window_scale => 2) do
  
  screen = Game.screen
  
  screen.clear
  
  sprites.each do |s|
    s.update
    screen.render_texture Sprite::TEXTURE, s.x, s.y
  end
  
  screen.render_text("Keyboard:", 8, 8, font, color)
  screen.render_text(Input.pressed_keys(:keyboard).inspect, 8 + 16, 8 + 16, font, color)
  
  screen.render_text("Gamepad:", 8, 8 + 32, font, color)
  screen.render_text(Input.pressed_keys(:gamepad).inspect, 8 + 16, 8 + 48, font, color)
  
  screen.render_text("Mouse:", 8, 8 + 64, font, color)
  screen.render_text(Input.pressed_keys(:mouse).inspect, 8 + 16, 8 + 80, font, color)
  x, y = Input.mouse_location
  screen.render_text("(#{x}, #{y})", x, y, font, color)
  
  screen.render_text("BGM Position:", 8, 8 + 96, font, color)
  screen.render_text(Audio.bgm_position.inspect, 8 + 16, 8 + 112, font, color)
  
  i += 1
  i %= 10
  Game.title = "%0.2f" % Game.real_fps if i == 0
  
  if Input.pressed_keys(:mouse, :duration => 1).include?(:left)
    unless Audio.playing_bgm?
      Audio.play_bgm("sounds/Mozart_-_Concerto_in_D_for_Flute_K.314.ladybyron", :volume => 16, :position => bgm_position, :time => 0)
    else
      bgm_position = Audio.bgm_position
      Audio.stop_bgm(:time => 0)
    end
  end

end
