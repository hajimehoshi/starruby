require "../starruby"

include StarRuby

texture = Texture.load("images/ruby")
texture.change_hue(Math::PI * 2 / 3)

Game.run do
  Screen.offscreen.fill Color.new(100, 110, 120, 130)
  Screen.offscreen.render_texture(texture, 0, 0, {
  })
end
