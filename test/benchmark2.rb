# ref: http://dgames.jp/dan/?permalink&date=20080417

require "starruby"
require "benchmark.rb"
include StarRuby

texture = Texture.load("images/star")

opts = {}
case ARGV.shift
when 'alpha'
  opts = { :alpha => 100 }
when 'angle'
  opts = { :angle => 0.314 }
when 'scale'
  opts = { :scale_x => 2.0, :scale_y => 2.0 }
end

x, y = 50, 50
render = lambda do
  Game.screen.render_texture(texture, x, y, opts)
end

Game.fps = 1000
counter = 0
Benchmark.bm do |b|
  b.report do
    Game.run(256, 256) do
      Game.screen.clear
      5000.times(&render)
      break if (counter += 1) >= 60
    end
  end
end
