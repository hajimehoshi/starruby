require "../starruby"

include StarRuby

font = Font.new("msgothic", 12, :ttc_index => 0)
color = Color.new(255, 155, 200, 255)

Game.run do
  # Screen.offscreen.fill Color.new(100, 110, 120, 130)
  Screen.offscreen.fill(Color.new(0, 0, 0, 0))
  Screen.offscreen.render_text("nnn\tn", 0, 0, font, color)
end
