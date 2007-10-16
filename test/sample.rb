require "../starruby"

include StarRuby

texture = Texture.load("images/ruby")

Game.run do
  Screen.offscreen.fill Color.new(100, 110, 120, 130)
  Screen.offscreen.render_texture(texture, 100, 11, {
    :tone_blue => 255
  })
end
