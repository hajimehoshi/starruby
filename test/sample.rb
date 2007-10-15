require "../starruby"

include StarRuby

texture = Texture.load("images/ruby")

Game.run do
  Screen.offscreen.fill Color.new(128, 128, 128, 128)
  Screen.offscreen.render_texture(texture, 0, 0, {
    # :alpha => 128#, :blend_type => :add
  })
end
