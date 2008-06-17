require "starruby"
require "falling_blocks/model"
require "falling_blocks/controller"
require "falling_blocks/view"

def main
  model      = FallingBlocks::Model.new(ARGV[0].to_i)
  controller = FallingBlocks::Controller.new(model)
  view       = FallingBlocks::View.new(model)
  StarRuby::Game.title = "Falling Blocks Game"
  StarRuby::Game.run(320, 240, :window_scale => 1) do
    Game.terminate if Input.keys(:keyboard).include?(:escape)
    controller.update
    view.update(StarRuby::Game.screen)
    GC.start
  end
end

main
