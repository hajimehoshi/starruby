require "starruby"
require "falling_blocks/model"
require "falling_blocks/controller"
require "falling_blocks/view"

def main
  model      = FallingBlocks::Model.new(ARGV[0].to_i)
  controller = FallingBlocks::Controller.new(model)
  view       = FallingBlocks::View.new(model)
  StarRuby::Game.run(320, 240,
                     :title => "Falling Blocks Game",
                     :window_scale => 1) do |game|
    break if Input.keys(:keyboard).include?(:escape)
    controller.update
    view.update(game.screen)
    GC.start
  end
end

main
