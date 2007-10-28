require "../starruby"

include StarRuby

font = Font.new("msgothic", 12, :ttc_index => 0)
color = Color.new(255, 155, 200, 255)

Game.run do
  Game.screen.fill(Color.new(0, 0, 0, 0))
  x, y = Input.mouse_location
  Game.screen.render_text([x, y].inspect, x, y, font, color)
end
