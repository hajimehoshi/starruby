require "../starruby"

include StarRuby

texture = Texture.load("images/ruby")

Game.run do
  Screen.offscreen.clear
  Screen.offscreen.render_texture(texture, 0, 0)
end
