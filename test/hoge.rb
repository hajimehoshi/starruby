require "starruby"
include StarRuby

# texture = Texture.load("images/ruby8_16colors")
texture = Texture.load("images/ruby16")

Game.run(320, 240, :window_scale => 2) do
  s = Game.screen
  s.fill(Color.new(128, 128, 128))
  s.render_texture(texture, 0, 0)
end
