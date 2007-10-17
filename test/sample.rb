require "../starruby"

include StarRuby

font = Font.new("arial", 12)
color = Color.new(255, 255, 255, 255)

Game.run do
  # Screen.offscreen.fill Color.new(100, 110, 120, 130)
  Screen.offscreen.fill(Color.new(0, 0, 0, 0))
  Screen.offscreen.render_text("hoge", 0, 0, font, color)
end
