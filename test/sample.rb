require "../starruby"

include StarRuby

texture = Texture.load("images/ruby")

Game.run do
  Screen.offscreen.clear
  Screen.offscreen.render_texture(texture, 10, 11, {
    :src_x => 12, :src_y => 13, :src_width => 14, :src_height => 15,
  })
end
