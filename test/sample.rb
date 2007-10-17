require "../starruby"

include StarRuby

texture = Texture.load("images/ruby")
texture.render_texture(texture, 10, 10)

Game.run do
  Screen.offscreen.fill Color.new(100, 110, 120, 130)
  Screen.offscreen.render_texture(texture, 0, 0)
end
